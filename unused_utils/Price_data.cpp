#include <iostream>
#include <string>
#include <curl/curl.h>
#include "../json.hpp" // Include nlohmann/json header

using json = nlohmann::json;

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

int main() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        // Set the URL for the API request
        std::string url = "https://api.energidataservice.dk/dataset/Elspotprices?"
                          "start=2022-05-01&end=2022-05-02&"
                          "filter={\"PriceArea\":[\"DK1\"]}&"
                          "limit=24";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        // Check for errors
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        // Clean up
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    // Parse the JSON data
    auto jsonData = json::parse(readBuffer);

    // Access and print specific data from the JSON
    for (const auto& record : jsonData["records"]) {
        std::string hourUTC = record["HourUTC"];
        double spotPriceDKK = record["SpotPriceDKK"];
        std::cout << "HourUTC: " << hourUTC << ", SpotPriceDKK: " << spotPriceDKK << std::endl;
    }

    return 0;
}
