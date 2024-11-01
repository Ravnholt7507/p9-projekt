#include <iostream>
#include <iomanip>
#include "vector"
#include "aggregation.h"
#include "../flexoffers/flexoffer.h"
using namespace std;

AggregatedFlexOffer::AggregatedFlexOffer(int offer_id, vector<Flexoffer> offers){
    id = offer_id;
    duration = 24;

    for (unsigned int i=0; i < offers.size(); i++){
        aggregated_earliest = min(aggregated_earliest, offers[i].earliest_start_time);
        aggregated_latest_start = max(aggregated_latest_start, offers[i].latest_start_time);
        aggregated_end_time = max(aggregated_end_time, offers[i].end_time);

        for (int j = 0; j < 24; j++ ){
            aggregated_profile[j].min_power += offers[i].profile[j].min_power;
            aggregated_profile[j].max_power += offers[i].profile[j].max_power;
        }
    }
    
    for(unsigned int i = 0; i < offers.size(); i++){
        individual_offers.push_back(offers[i]);
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

    cout << "=== FlexOffer Details ===" << endl;
    cout << "Offer ID: " << id << endl;
    cout << "Earliest Start Time: " << to_readable(aggregated_earliest) << endl;
    cout << "Latest Start Time: " << to_readable(aggregated_latest_start) << endl;
    cout << "Latest End Time:     " << to_readable(aggregated_end_time) << endl;
    cout << "Duration:            " << duration << " hour(s)" << endl;
    cout << "Profile Elements:" << endl;

    for(int i = 0; i < 24; i++) {
        if(aggregated_profile[i].min_power > 0 || aggregated_profile[i].max_power > 0) {
            cout << "  Hour " << i << ": Min Power = " << fixed << setprecision(2) 
                 << aggregated_profile[i].min_power << " kW, Max Power = " 
                 << aggregated_profile[i].max_power << " kW" << endl;
        }
    }
    cout << "==========================" << endl;
};
