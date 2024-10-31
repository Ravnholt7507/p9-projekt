#include "vector"
#include "aggregation.h"
#include "../flexoffers/flexoffer.h"
using namespace std;

AggregatedFlexOffer::AggregatedFlexOffer(int offer_id, vector<Flexoffer> offers){
    id = offer_id;
    duration = 24;
    aggregated_earliest = offers[0].earliest_start_time;
    aggregated_end_time = offers[0].end_time;

    for (unsigned int i=1; i < offers.size(); i++){
        aggregated_earliest = min(aggregated_earliest, offers[i].earliest_start_time);
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
    
};
