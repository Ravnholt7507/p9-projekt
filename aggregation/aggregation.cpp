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

    }
    for (unsigned int i=0; i < offers.size(); i++){
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

    cout << "=== Aggregated FlexOffer Details ===" << endl;
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

void disaggregate(AggregatedFlexOffer AFO) {
    double scheduled_allocation_norm[24] = {0};

    for (int i = 0; i < 24; i++){
        if (AFO.scheduled_allocation[i] > 0.0) {
            scheduled_allocation_norm[i] = 
            (AFO.scheduled_allocation[i] - AFO.aggregated_profile[i].min_power) / 
            (AFO.aggregated_profile[i].max_power - AFO.aggregated_profile[i].min_power);
        }
    }

    for (auto &offer : AFO.individual_offers) {
        for (int i = 0; i < 24; i++){
            offer.scheduled_allocation[i] = offer.profile[i].min_power + ((offer.profile[i].max_power - offer.profile[i].min_power) * scheduled_allocation_norm[i]);
        }
    }

}