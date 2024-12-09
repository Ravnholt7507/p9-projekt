#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <random>

#include "../include/flexoffer.h"
#include "../include/tec.h"

using namespace std;

time_t generateRandomTimestampToday(); 
void printTimestamp(time_t timestamp); 
int randomInt(int min, int max); 
double randomDouble(double min, double max);

Flexoffer generateFlexOffer(int id) {
    time_t earliest_start_time = generateRandomTimestampToday();
    int time_flex = randomInt(1, 4);
    time_t latest_start_time = earliest_start_time + (time_flex * 3600);
    int duration = randomInt(1, 4);
    time_t end_time = latest_start_time + (duration * 3600);

    if (latest_start_time + (duration * 3600) > end_time) {
        latest_start_time = end_time - (duration * 3600);
    }

    tm *lst_tm = localtime(&latest_start_time);
    int latest_hour = lst_tm->tm_hour;

    if (latest_hour > (23 - duration)) {
        latest_hour = 23 - duration;
    }

    vector<TimeSlice> profile(duration);
    

    for (int i = 0; i < duration; i++) {
        profile[i].min_power = randomDouble(0.5, 1.0);
        profile[i].max_power = randomDouble(1.0, 3.0);
    }
    
    // Create and return the Flexoffer object
    Flexoffer obj(id, earliest_start_time, latest_start_time, end_time, profile, duration);

    return obj;
}


Tec_flexoffer generateTecFlexOffer(int id) {
    time_t earliest_start_time = generateRandomTimestampToday();
    int time_flex = randomInt(1, 4);
    time_t latest_start_time = earliest_start_time + (time_flex * 3600);
    int duration = randomInt(1, 4);
    time_t end_time = latest_start_time + (duration * 3600);

    if (latest_start_time + (duration * 3600) > end_time) {
        latest_start_time = end_time - (duration * 3600);
    }

    tm *lst_tm = localtime(&latest_start_time);
    int latest_hour = lst_tm->tm_hour;

    if (latest_hour > (23 - duration)) {
        latest_hour = 23 - duration;
    }

    vector<TimeSlice> profile(duration);
    

    for (int i = 0; i < duration; i++) {
        profile[i].min_power = randomDouble(0.5, 1.0);
        profile[i].max_power = randomDouble(1.0, 3.0);
    }

    double min = 0;
    double max = 14;
    
    // Create and return the Flexoffer object
    Tec_flexoffer obj(min, max, id, earliest_start_time, latest_start_time, end_time, profile, duration);

    return obj;
}

vector<Flexoffer> generateMultipleFlexOffers(int numOffers) {
    vector<Flexoffer> flexOffers;
    for (int i = 1; i <= numOffers; i++) {
        Flexoffer flexOffer = generateFlexOffer(i); // ID starts from 1
        flexOffers.push_back(flexOffer); // Add each Flexoffer to the vector
    }
    return flexOffers;
}

vector<Tec_flexoffer> generateMultipleTecFlexOffers(int numOffers) {
    vector<Tec_flexoffer> flexOffers;
    for (int i = 1; i <= numOffers; i++) {
        Tec_flexoffer flexOffer = generateTecFlexOffer(i); // ID starts from 1
        flexOffers.push_back(flexOffer); // Add each Flexoffer to the vector
    }
    return flexOffers;
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

    // Generate a random number of hours to add (0 to 23)
    int hour_dist = randomInt(0,18);
    // Add random hours to the start of the day
    time_t random_timestamp = start_of_day + (hour_dist * 3600);

    return random_timestamp;
}

void printTimestamp(time_t timestamp) {
    tm local_tm = *localtime(&timestamp);
    cout << put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
}


static random_device rd;
static mt19937 gen(rd());

int randomInt(int min, int max) {
     // Random number generator seeded with random_device
    uniform_int_distribution<> dist(min, max);

    // Generate and return a random integer between min and max
    return dist(gen);
}

double randomDouble(double min, double max) {
    uniform_real_distribution<> dist(min, max);
    return dist(gen);
}
