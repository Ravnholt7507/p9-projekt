#include <iostream>
#include <ctime>
#include <iomanip>
#include <vector>

#include "../include/aggregation.h"
#include "../include/flexoffer.h"
using namespace std;
//Constructor
AggregatedFlexOffer::AggregatedFlexOffer(int offer_id, vector<Flexoffer> &offers){
  //  individual_offers = offers;
    id = offer_id;

    //step 1. Assign earliest and latest times of aggregated flex offer. (by looking at the min/max of individual offers)
    for (unsigned int i=0; i < offers.size(); i++){
        aggregated_earliest = min(aggregated_earliest, offers[i].get_est());
        aggregated_latest = min(aggregated_latest, offers[i].get_lst() - offers[i].get_est());
        aggregated_end_time = max(aggregated_end_time, offers[i].get_est()+ (offers[i].get_duration() * 3600));
    }
    aggregated_latest += aggregated_earliest;

    // Conversion to hours instead of seconds, needed in the nested loop below
    int aggregated_end_time_hour = localtime(&aggregated_end_time)->tm_hour;
    int aggregated_earliest_hour = localtime(&aggregated_earliest)->tm_hour;
    duration = aggregated_end_time_hour - aggregated_earliest_hour;


    aggregated_profile.resize(24);
    //step 2. Loop through our offers and aggregate on each timeslice.
    for (int j = aggregated_earliest_hour; j < aggregated_end_time_hour; j++) {
        TimeSlice slice = {0.0, 0.0};

        for (auto& offer : offers) {
            time_t est = offer.get_est();
            int offer_start_hour = localtime(&est)->tm_hour;
            if (j >= offer_start_hour && j < offer_start_hour + offer.get_duration()) {
                int relative_index = j - offer_start_hour;
                slice.min_power += offer.get_profile()[relative_index].min_power;
                slice.max_power += offer.get_profile()[relative_index].max_power;
            }
        }
        // Assign the aggregated time slice to the profile
        aggregated_profile[j] = slice;
    }
};

//Getters
int AggregatedFlexOffer::get_id() const {return id;};
time_t AggregatedFlexOffer::get_aggregated_earliest() const {return aggregated_earliest;};
time_t AggregatedFlexOffer::get_aggregated_latest() const {return aggregated_latest;};
time_t AggregatedFlexOffer::get_aggregated_end_time() const {return aggregated_end_time;};
vector<TimeSlice> AggregatedFlexOffer::get_aggregated_profile() const {return aggregated_profile;};
double* AggregatedFlexOffer::get_scheduled_allocation() {return scheduled_allocation;};
int AggregatedFlexOffer::get_duration() const {return duration;}; 
vector<Flexoffer> AggregatedFlexOffer::get_individual_offers() const {return individual_offers;};

//Setters
void AggregatedFlexOffer::set_id(int value) {id = value;};
void AggregatedFlexOffer::set_aggregated_earliest(time_t value) {aggregated_earliest = value;};
void AggregatedFlexOffer::set_aggregated_latest(time_t value) {aggregated_latest = value;};
void AggregatedFlexOffer::set_aggregated_end_time(time_t value) {aggregated_end_time = value;};
void AggregatedFlexOffer::set_aggregated_profile(vector<TimeSlice> value) {aggregated_profile = value;};
void AggregatedFlexOffer::set_scheduled_allocation(double value[24]) {
    for(int i = 0; i < 24; i++){
        scheduled_allocation[i] = value[i];
    } 
};
void AggregatedFlexOffer::set_duration(int value) {duration = value;}; 
void AggregatedFlexOffer::set_individual_offers(vector<Flexoffer> value) {individual_offers = value;};

//Utils
void AggregatedFlexOffer::pretty_print(){
    // Helper lambda to convert time_t to readable format
    auto to_readable = [](time_t timestamp) -> string {
        char buffer[20];
        struct tm * timeinfo = localtime(&timestamp);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return string(buffer);
    };

    cout << "=== Aggregated FlexOffer Details ===" << endl;
    cout << "Offer ID: " << id << endl;
    cout << "Earliest Start Time: " << to_readable(aggregated_earliest) << endl;
    cout << "Latest Start Time: " << to_readable(aggregated_latest) << endl;
    cout << "Latest End Time:     " << to_readable(aggregated_end_time) << endl;
    cout << "Duration:            " << duration << " hour(s)" << endl;
    cout << "Profile Elements:" << endl;

    // int aggregated_end_time_hour = localtime(&aggregated_end_time)->tm_hour;
    // int aggregated_earliest_hour = localtime(&aggregated_earliest)->tm_hour;    

    for(int i = 0; i < 24; i++) {
        if(aggregated_profile[i].min_power > 0.0 || aggregated_profile[i].max_power > 0.0) {
        cout << "  Hour " << i << ": Min Power = " << fixed << setprecision(2) 
            << aggregated_profile[i].min_power << " kW, Max Power = " 
            << aggregated_profile[i].max_power << " kW" << endl;
        }
    }
    cout << "==========================" << endl;
};



void AggregatedFlexOffer::schedule(){
    for (int i=0; i < 24; i++){
        scheduled_allocation[i] = (aggregated_profile[i].min_power + aggregated_profile[i].max_power) / 2.0;
    }

    cout << "Scheduled allocation: " << "\n";
    for(int i = 0; i < 24; i++) {
        if(aggregated_profile[i].min_power > 0 || aggregated_profile[i].max_power > 0) {
            cout << "  Hour " << i << ": scheduled Power = " << fixed << setprecision(2) << scheduled_allocation[i] << "\n";
        }
    }
};
