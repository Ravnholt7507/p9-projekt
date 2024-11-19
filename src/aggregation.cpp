#include <iostream>
#include <ctime>
#include <iomanip>
#include <vector>

#include "../include/aggregation.h"
#include "../include/flexoffer.h"
using namespace std;

AggregatedFlexOffer::AggregatedFlexOffer(int offer_id, vector<Flexoffer> &offers){
  //  individual_offers = offers;
    id = offer_id;

    //step 1. Assign earliest and latest times of aggregated flex offer. (by looking at the min/max of individual offers)
    for (unsigned int i=0; i < offers.size(); i++){
        aggregated_earliest = min(aggregated_earliest, offers[i].earliest_start_time);
        aggregated_latest = min(aggregated_latest, offers[i].latest_start_time - offers[i].earliest_start_time);
        aggregated_end_time = max(aggregated_end_time, offers[i].earliest_start_time + (offers[i].duration * 3600));
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
            int offer_start_hour = localtime(&offer.earliest_start_time)->tm_hour;
            if (j >= offer_start_hour && j < offer_start_hour + offer.duration) {
                int relative_index = j - offer_start_hour;
                slice.min_power += offer.profile[relative_index].min_power;
                slice.max_power += offer.profile[relative_index].max_power;
            }
        }
        // Assign the aggregated time slice to the profile
        aggregated_profile[j] = slice;
    }
};

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
