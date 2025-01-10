#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include "../json.hpp" // Include nlohmann/json header
#include <stdexcept>
#include <sstream>
#include <ctime>

using namespace std;
using json = nlohmann::json;


// Callback function to handle data received from libcurl
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string timeToGmtString(time_t t)
{
    char buf[128];
    std::tm *gmt = std::gmtime(&t);
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return std::string(buf);
}


time_t forceDate(time_t origTime){
    std::tm localTm = *std::gmtime(&origTime);

    localTm.tm_year = 2019 - 1900;  // 2019
    localTm.tm_mon  = 4;
    localTm.tm_mday = 20;

    return timegm(&localTm);
}

time_t parseDateTime(const std::string &datetimeStr) {
    std::tm timeinfo = {};
    std::istringstream iss(datetimeStr);
    iss >> std::get_time(&timeinfo, "%a, %d %b %Y %H:%M:%S GMT");
    if (iss.fail()) {
        throw std::runtime_error("Invalid date-time format: " + datetimeStr);
    }
    return timegm(&timeinfo); 
}

void downloadData(const string &url, const string &outputFilePath)
{
    CURL* curl;
    CURLcode res;
    string readBuffer;
    
    // Replace with your actual token
    string token = "-YEA7_noT4td5L8Vp8cbgjGzwDlCxP1r4a7Ig1iT8cU";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        struct curl_slist* headers = nullptr;
        string authHeader = "Authorization: Bearer " + token;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform failed: " << curl_easy_strerror(res) << endl;
        } else {
            // Clean up curl
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            // Parse the received JSON
            auto jsonData = json::parse(readBuffer);

            // Open the output CSV file in append mode
            ofstream outFile(outputFilePath, ios::app);
            if (outFile.is_open()) {
                static bool headerWritten = false;

                // Write the header only once
                if (!headerWritten && !jsonData["_items"].empty()) {
                    for (auto& field : jsonData["_items"][0].items()) {
                        outFile << field.key() << ",";
                    }
                    outFile.seekp(-1, ios_base::cur); // Remove trailing comma
                    outFile << "\n";
                    headerWritten = true;
                }

                // Extract and write all fields for each record
                for (const auto& record : jsonData["_items"]) {
                    try {
                        // 1) Make sure these fields exist
                        if (!record.contains("connectionTime") || !record.contains("doneChargingTime")) {
                            // If they're missing, skip
                            continue;
                        }

                        // 2) Parse them as time_t
                        const string connTimeStr = record["connectionTime"].get<string>();
                        const string doneTimeStr = record["doneChargingTime"].get<string>();
                        const string disTimeStr = record["disconnectTime"].get<string>();

                        time_t connTime  = parseDateTime(connTimeStr);
                        time_t doneTime  = parseDateTime(doneTimeStr);
                        time_t disTime  = parseDateTime(disTimeStr);

                        // 3) Check duration (skip if < 1 hour)
                        double diffSec = difftime(doneTime, connTime);
                        double diffSec2 = difftime(disTime, connTime);
                        double diff_done_dis = difftime(disTime, doneTime);
                        if (diffSec < 3600.0 || diffSec2 < 3600 || diff_done_dis < 0) {
                            continue;
                        }

                        // 4) Keep **only** if they occur on the SAME day (skip if crossing days)
                        //    We'll compare (year, month, day) in UTC
                        struct tm connTM = *gmtime(&connTime);
                        struct tm doneTM = *gmtime(&doneTime);
                        struct tm disTM = *gmtime(&disTime);

                        // If they differ by day, skip
                        if (connTM.tm_year != doneTM.tm_year || connTM.tm_mon  != doneTM.tm_mon  || connTM.tm_mday != doneTM.tm_mday || 
                            connTM.tm_year != disTM.tm_year || connTM.tm_mon  != disTM.tm_mon  || connTM.tm_mday != disTM.tm_mday ||
                            disTM.tm_year != doneTM.tm_year || disTM.tm_mon  != doneTM.tm_mon  || disTM.tm_mday != doneTM.tm_mday ){
                            continue;
                        }

                        time_t forcedConn = forceDate(connTime);
                        time_t forcedDone = forceDate(doneTime);
                        time_t forcedDisc = forceDate(disTime);

                        json modifiedRec = record;
                        modifiedRec["connectionTime"]   = timeToGmtString(forcedConn);
                        modifiedRec["doneChargingTime"] = timeToGmtString(forcedDone);
                        modifiedRec["disconnectTime"]   = timeToGmtString(forcedDisc);

                        for (auto &f : modifiedRec.items()) {
                            outFile << (f.value().is_null() ? "" : f.value().dump()) << ",";
                        }
                        // Remove trailing comma
                        outFile.seekp(-1, ios_base::cur);
                        outFile << "\n";

                    } catch (const std::exception &e) {
                        cerr << "Record parse error: " << e.what() << endl;
                        // skip this record
                        continue;
                    }
                }

                outFile.close();
                cout << "Data appended to " << outputFilePath << endl;
            } else {
                cerr << "Error opening file for writing: " << outputFilePath << endl;
            }
        }
    }
}

int main() {
    int totalPages = 50;
    string outputfilePath = "../data/ev_data.csv";

    for (int page = 1; page <= totalPages; ++page) {
        ostringstream apiURL;
        apiURL 
            << "https://ev.caltech.edu/api/v1/sessions/caltech?"
            << "where=connectionTime%3E%3D%22Mon,%2020%20May%202019%2000:00:00%20GMT%22%20"
            << "and%20connectionTime%3C%22Mon,%2003%20Jun%202019%2000:00:00%20GMT%22"
            << "&page=" << page
            << "&pretty";
        downloadData(apiURL.str(), outputfilePath);
    }
    return 0;
}
