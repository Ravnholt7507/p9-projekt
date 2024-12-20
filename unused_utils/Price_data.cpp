#include <iostream>
#include <string>
#include <curl/curl.h>
#include "../json.hpp" // Include nlohmann/json header
#include <fstream> // For file operations

using namespace std;
using json = nlohmann::json;


void writeSpotPricesToCSV(const std::vector<double>& spotPrices, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    // Write header
    outFile << "SpotPriceDKK\n";

    // Write data
    for (const auto& price : spotPrices) {
        outFile << price << '\n';
    }

    outFile.close();
    if (outFile.fail()) {
        std::cerr << "Error closing file: " << filename << std::endl;
    }
}


// Callback function to handle data received from libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}

// Function to download and process price data
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
                outFile << "SpotPriceEURperKWh\n";

                // Extract and convert data
                for (const auto& record : jsonData["records"]) {
                    if (record.contains("HourUTC") && record.contains("SpotPriceEUR")) {
                        string hourUTC = record["HourUTC"].get<string>();
                        double priceEURPerMWh = record["SpotPriceEUR"].get<double>();

                        // Convert from EUR/MWh to EUR/kWh
                        double priceEURPerKWh = priceEURPerMWh / 1000.0;

                        // Write to CSV
                        outFile << priceEURPerKWh << "\n";
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
    string apiURL = "https://api.energidataservice.dk/dataset/Elspotprices?"
                    "start=2022-05-01&end=2022-05-02&"
                    "filter={\"PriceArea\":[\"DK1\"]}&"
                    "limit=24";

    string outputFilePath = "converted_prices.csv";

    downloadAndConvertPrices(apiURL, outputFilePath);

    return 0;
}