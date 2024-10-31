#ifndef AGGREGATION_H
#define AGGREGATION_H
#include <vector>
#include "../flexoffers/flexoffer.h"


class AggregatedFlexOffer{
    public:
        int id;
        time_t aggregated_earliest;
        time_t aggregated_end_time;
        TimeSlice aggregated_profile[24] = {0};
        int duration; 
        vector<Flexoffer> individual_offers;
        AggregatedFlexOffer(int offer_id, vector<Flexoffer> offers);
        void pretty_print();
};


#endif
