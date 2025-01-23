#include <iostream>
#include <string>
#include <curl/curl.h>
#include "../json.hpp"
#include <fstream>

using namespace std;
using json = nlohmann::json;


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}

void downloadAndConvertPrices(const string &url, const string &outputFilePath) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform failed: " << curl_easy_strerror(res) << endl;
        } else {
            curl_easy_cleanup(curl);

            // Parse JSON data
            auto jsonData = json::parse(readBuffer);

            // Open the output CSV file
            ofstream outFile(outputFilePath);
            if (outFile.is_open()) {
                // Write header
                outFile << "FCRdomestic_MW,FCRabroad_MW,FCRcross_EUR,FCRdk_EUR\n";

                // Extract and convert data
                for (const auto& record : jsonData["records"]) {
                    if (record.contains("HourUTC")) {
                        string hourUTC = record["HourUTC"].get<string>();
                        double fcr_dom = record["FCRdomestic_MW"].get<double>();
                        double fcr_abr = record["FCRabroad_MW"].get<double>();
                        double fcr_price_cross = record["FCRcross_EUR"].get<double>();
                        double fcr_price_dk = record["FCRdk_EUR"].get<double>();

                        // Convert from EUR/MWh to EUR/kWh
                        fcr_dom = fcr_dom / 1000.0;
                        fcr_abr = fcr_abr / 1000.0;
                        fcr_price_cross = fcr_price_cross / 1000.0;
                        fcr_price_dk = fcr_price_dk  / 1000.0;

                        // Write to CSV
                        outFile << fcr_dom << "," << fcr_abr << "," << fcr_price_cross << "," << fcr_price_dk << "\n";
                    }
                }

                outFile.close();
                cout << "Converted data saved to " << outputFilePath << endl;
            } else {
                cerr << "Error opening file for writing: " << outputFilePath << endl;
            }
        }
    }
}

int main() {
    string apiURL = "https://api.energidataservice.dk/dataset/FcrDK1?offset=0&start=2022-05-01T00:00&end=2022-05-02T00:00&sort=HourUTC%20ASC";

    string outputFilePath = "../data/FCRprices.csv";

    downloadAndConvertPrices(apiURL, outputFilePath);

    return 0;
}