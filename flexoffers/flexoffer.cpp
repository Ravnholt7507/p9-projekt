#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include "random"
#include "flexoffer.h"
using namespace std;

time_t generateRandomTimestampToday(); 
void printTimestamp(time_t timestamp); 
int randomInt(int min, int max); 

Flexoffer::Flexoffer(int oi, time_t est, time_t lst, double *p, int d, time_t et){
        offer_id = oi;
        earliest_start_time = est;
        latest_start_time = lst;
        for(int i = 0; i < 24; i++) {
            profile[i] = p[i];
        }
        duration = d;
        end_time = et;
};

void Flexoffer::print_flexoffer(){
    cout << "Offer id: " << offer_id << endl << "Earliest start time: ";
    printTimestamp(earliest_start_time);
    cout << endl << "latest_start_time: ";
    printTimestamp(latest_start_time);
    cout << endl << "Profile elements: ";
    for(int i = 0; i < 24; i++){
        std::cout << profile[i] << " ";
    }
    cout << endl << "Duration: " << duration << endl << "End time: ";
    printTimestamp(end_time);
    cout << endl;
}

Flexoffer generateFlexOffer(int id) {
    time_t earliest_start_time = generateRandomTimestampToday();
    double kwh = 10;
    int duration = randomInt (1, 4);
    double kwh_per_hour = kwh / duration;
    int start_time  = randomInt(0, 23-duration);
    double profile[24] = {0};
    time_t latest_end_time = earliest_start_time + (60*60*duration);
    time_t end_time = earliest_start_time + (60*60*24);

    for(int i = start_time; i < start_time + duration; i++){
        profile[i] = kwh_per_hour;
    }

    
    Flexoffer obj(id, earliest_start_time, latest_end_time, profile, duration, end_time);

    return obj;
}

time_t generateRandomTimestampToday() {
    // Get the current time and date
    auto now = chrono::system_clock::now();
    time_t now_t = chrono::system_clock::to_time_t(now);

    // Convert to local time and set to midnight (start of the day)
    tm local_tm = *localtime(&now_t);
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
    tm local_tm = *localtime(&timestamp);
    cout << put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
}


int randomInt(int min, int max) {
     // Random number generator seeded with random_device
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min, max);

    // Generate and return a random integer between min and max
    return dist(gen);
}
