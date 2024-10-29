#include "iostream"
#include "fstream"
#include <ctime>
#include <chrono>
#include <iomanip>
#include "flexoffers/flexoffer.h"
#include "curl/curl.h"
#include "random"

using namespace std;

time_t generateRandomTimestampToday() {
    // Get the current time and date
    auto now = chrono::system_clock::now();
    time_t now_t = chrono::system_clock::to_time_t(now);

    // Convert to local time and set to midnight (start of the day)
    tm local_tm = localtime(&now_t);
    local_tm.tm_hour = 0;
    local_tm.tm_min = 0;
    local_tm.tm_sec = 0;
    time_t start_of_day = mktime(&local_tm);

    // Generate a random number of seconds to add (0 to 86399)
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 86399); // Total seconds in a day minus one

    // Add random seconds to the start of the day
    time_t random_timestamp = start_of_day + dist(gen);

    return random_timestamp;
}

void printTimestamp(time_t timestamp) {
    tm local_tm = localtime(&timestamp);
    cout << put_time(local_tm, "%Y-%m-%d %H:%M:%S") << endl;
}


int randomInt(int min, int max) {
    return min + rand() % (max - min);
}

Flexoffer generateFlexOffer(int id) {
    time_t earliest_start_time = generateRandomTimestampToday();
    int flexibility_window = randomInt(1, 5);
    int duration = randomInt (1, 4);
    time_t latest_end_time = earliest_start_time + duration



}

