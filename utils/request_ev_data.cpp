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


time_t parseDateTime(const std::string &datetimeStr) {
    // We'll assume it's something like "Wed, 25 Apr 2018 11:08:04 GMT"
    // Adjust as necessary for your exact format
    std::tm timeinfo = {};
    std::istringstream iss(datetimeStr);
    // Example: "Wed, 25 Apr 2018 11:08:04 GMT"
    iss >> std::get_time(&timeinfo, "%a, %d %b %Y %H:%M:%S GMT");
    if (iss.fail()) {
        // If it fails, throw or return 0
        throw std::runtime_error("Invalid date-time format: " + datetimeStr);
    }
    // Convert from struct tm to time_t (UTC)
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

                        time_t connTime  = parseDateTime(connTimeStr);
                        time_t doneTime  = parseDateTime(doneTimeStr);

                        // 3) Check duration (skip if < 1 hour)
                        double diffSec = difftime(doneTime, connTime);
                        if (diffSec < 3600.0) {
                            continue;
                        }

                        // 4) Keep **only** if they occur on the SAME day (skip if crossing days)
                        //    We'll compare (year, month, day) in UTC
                        struct tm connTM = *gmtime(&connTime);
                        struct tm doneTM = *gmtime(&doneTime);

                        // If they differ by day, skip
                        if (connTM.tm_year != doneTM.tm_year ||
                            connTM.tm_mon  != doneTM.tm_mon  ||
                            connTM.tm_mday != doneTM.tm_mday)
                        {
                            continue;
                        }

                        // If we get here, this record meets all criteria
                        for (auto &field : record.items()) {
                            // If the field value is null, write an empty string
                            outFile << (field.value().is_null() ? "" : field.value().dump()) << ",";
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
    int totalPages = 2;
    string outputfilePath = "../data/ev_data.csv";

    for (int page = 1; page <= totalPages; ++page) {
        ostringstream apiURL;
        apiURL << "https://ev.caltech.edu/api/v1/sessions/caltech?"
               << "where=connectionTime%3E%3D%22Mon,%2020%20May%202019%2000:00:00%20GMT%22%20"
               << "and%20connectionTime%3C%22Tue,%2021%20May%202019%2000:00:00%20GMT%22"
               << "&page=" << page
               << "&pretty";
        downloadData(apiURL.str(), outputfilePath);
    }
    return 0;
}
