#include <iostream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <string>

#include "../include/tec.h"
#include "../include/flexoffer.h"

using namespace std;

//Constructor
Tec_flexoffer::Tec_flexoffer(double min, double max, int oi, time_t est, time_t lst, time_t et, vector<TimeSlice> &p, int d) : Flexoffer(oi, est, lst, et, p, d) {
    min_overall_kw = min;     
    max_overall_kw = max;
};
//Deconstructor
Tec_flexoffer::~Tec_flexoffer(){
   min_overall_kw = 0;
   max_overall_kw = 0; 
};

//Getters
double Tec_flexoffer::get_min_overall_kw() const {return min_overall_kw;};
double Tec_flexoffer::get_max_overall_kw() const {return max_overall_kw;};

//Setters
void Tec_flexoffer::set_min_overall_kw(double value) {min_overall_kw = value;};
void Tec_flexoffer::set_max_overall_kw(double value) {max_overall_kw = value;};

//overrides
void Tec_flexoffer::print_flexoffer() {
    // Helper lambda to convert time_t to readable format
    auto to_readable = [](time_t timestamp) -> string {
        char buffer[20];
        struct tm * timeinfo = localtime(&timestamp);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return string(buffer);
    };

    cout << "=== FlexOffer Details ===" << endl;
    cout << "Offer ID: " << get_offer_id() << endl;
    cout << "Earliest Start Time: " << to_readable(get_est()) << endl;
    cout << "Latest Start Time:   " << to_readable(get_lst()) << endl;
    cout << "Latest End Time:     " << to_readable(get_et()) << endl;
    cout << "Min Overall kw:      " << get_min_overall_kw() << endl;
    cout << "Max Overall kw:      " << get_max_overall_kw() << endl;
    cout << "Duration:            " << get_duration() << " hour(s)" << endl;
    cout << "Profile Elements:" << endl;

    for(int i = 0; i < get_duration(); i++) {
        cout << "  Hour " << i << ": Min Power = " << fixed << setprecision(2) 
                << get_profile()[i].min_power << " kW, Max Power = " 
                << get_profile()[i].max_power << " kW" << endl;
        
    }
    cout << "==========================" << endl;
};
