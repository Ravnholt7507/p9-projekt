#include <ctime>
#include "vector"
#include "aggregation.h"
#include "../flexoffers/flexoffer.h"
using namespace std;

Flexoffer AggregateFlexOffers(vector<Flexoffer> offers){
    time_t aggregated_earliest = offers[0].earliest_start_time;
    time_t aggregated_end_time = offers[0].end_time;
    TimeSlice aggregatedprofile[24] = {0};

    for (unsigned int i=1; i < offers.size(); i++){
        aggregated_earliest = min(aggregated_earliest, offers[i].earliest_start_time);
        aggregated_end_time = max(aggregated_end_time, offers[i].earliest_start_time);

        for (int j = 0; j < 24; j++ ){
            aggregatedprofile[j].min_power += offers[i].profile[j].min_power;
            aggregatedprofile[j].max_power += offers[i].profile[j].max_power;
        }
    }

    Flexoffer obj(0, aggregated_earliest, aggregated_earliest, aggregated_end_time, aggregatedprofile, 24);
    return obj;
}
