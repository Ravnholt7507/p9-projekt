#include <iostream>
#include <ctime>
#include <iomanip>
#include <vector>

#include "../include/flexoffer.h"

using namespace std;

//Constructor
Flexoffer::Flexoffer(int oi, time_t est, time_t lst, time_t et, vector<TimeSlice> &p, int d){
    offer_id = oi;
    earliest_start_time = est;
    latest_start_time = lst;
    duration = d;
    profile = p;
    end_time = et;
};

//Getters
int Flexoffer::get_offer_id() const {return offer_id;};
time_t Flexoffer::get_est() const {return earliest_start_time;};
time_t Flexoffer::get_lst() const {return latest_start_time;};
time_t Flexoffer::get_et() const {return end_time;};
int Flexoffer::get_duration() const {return duration;};
vector<TimeSlice> Flexoffer::get_profile() const {return profile;};
vector<double> Flexoffer::get_scheduled_allocation() const {return scheduled_allocation;};
time_t Flexoffer::get_scheduled_start_time() const {return scheduled_start_time;};

//Setters
void Flexoffer::set_scheduled_allocation(vector<double> new_sa) {scheduled_allocation = new_sa;};
void Flexoffer::set_scheduled_start_time(time_t new_st) {scheduled_start_time = new_st;};

//Utils
void Flexoffer::print_flexoffer() {
    // Helper lambda to convert time_t to readable format
    auto to_readable = [](time_t timestamp) -> string {
        char buffer[20];
        struct tm * timeinfo = localtime(&timestamp);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return string(buffer);
    };

    cout << "=== FlexOffer Details ===" << endl;
    cout << "Offer ID: " << offer_id << endl;
    cout << "Earliest Start Time: " << to_readable(earliest_start_time) << endl;
    cout << "Latest Start Time:   " << to_readable(latest_start_time) << endl;
    cout << "Latest End Time:     " << to_readable(end_time) << endl;
    cout << "Duration:            " << duration << " hour(s)" << endl;
    cout << "Profile Elements:" << endl;

    for(int i = 0; i < duration; i++) {
        cout << "  Hour " << i << ": Min Power = " << fixed << setprecision(2) 
                << profile[i].min_power << " kW, Max Power = " 
                << profile[i].max_power << " kW" << endl;
        
    }
    cout << "==========================" << endl;
};
