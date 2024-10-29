#include <iostream>
#include <fstream>
#include <curl/curl.h>

using namespace std;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void downloadData(const string &url, const string &outputFilePath)
{
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            cerr << "curl easy perform failed: " << curl_easy_strerror(res) << endl;
        } 
        else {
            curl_easy_cleanup(curl);
            ofstream outFile(outputFilePath);
            if (outFile.is_open()){
                outFile << readBuffer;
                outFile.close();
                cout << "data saved to " << outputFilePath << endl;
            }
            else {
                cout << "error opening file for writing";
            }
        }
    }
}

int main() {
    string apiURL = "https://ev.caltech.edu/api/v1/sessions/caltech?";
    string outputfilePath = "ev_data.json";

    downloadData(apiURL, outputfilePath);
    return 0;
}