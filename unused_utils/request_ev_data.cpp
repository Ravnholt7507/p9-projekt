#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include "../json.hpp" // Include nlohmann/json header

using namespace std;
using json = nlohmann::json;

// Callback function to handle data received from libcurl
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to download and process data
void downloadData(const string &url, const string &outputFilePath)
{
    CURL* curl;
    CURLcode res;
    string readBuffer;
    string token = "-YEA7_noT4td5L8Vp8cbgjGzwDlCxP1r4a7Ig1iT8cU";

    curl = curl_easy_init();
    if(curl) {
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
        } 
        else {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            std::cout << "API Response: " << readBuffer << std::endl;
            auto jsonData = json::parse(readBuffer);

            // Open the output CSV file
            ofstream outFile(outputFilePath);
            if (outFile.is_open()) {
                // Extract keys for the header
                if (!jsonData["_items"].empty()) {
                    for (auto& field : jsonData["_items"][0].items()) {
                        outFile << field.key() << ",";
                    }
                    outFile.seekp(-1, ios_base::cur); // Remove trailing comma
                    outFile << "\n";
                }

                // Extract and write all fields for each record
                for (const auto& record : jsonData["_items"]) {
                    for (auto& field : record.items()) {
                        outFile << (field.value().is_null() ? "" : field.value().dump()) << ",";
                    }
                    outFile.seekp(-1, ios_base::cur); // Remove trailing comma
                    outFile << "\n";
                }

                outFile.close();
                cout << "Data saved to " << outputFilePath << endl;
            } else {
                cerr << "Error opening file for writing: " << outputFilePath << endl;
            }
        }
    }
}

int main() {
std::string apiURL = "https://ev.caltech.edu/api/v1/sessions/caltech?%20where=connectionTime%3E=%22Wed,%201%20May%202019%2000:00:00%20GMT%22%20and%20connectionTime%3C=%22Thu,%202%20May%202019%2000:00:00%20GMT%22%20&pretty";

    string outputfilePath = "../data/ev_data.csv";

    downloadData(apiURL, outputfilePath);
    return 0;
}