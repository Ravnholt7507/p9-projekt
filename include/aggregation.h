#ifndef AGGREGATION_H
#define AGGREGATION_H
#include <ctime>
#include <limits>
#include <vector>
#include <chrono>

#include "../include/flexoffer.h"

using namespace std;

class AggregatedFlexOffer{
    public:
        int id;
        time_t aggregated_earliest = std::numeric_limits<int>::max();
        time_t aggregated_latest= std::numeric_limits<int>::max();
        time_t aggregated_end_time = 0;
        vector<TimeSlice> aggregated_profile;
        double scheduled_allocation[24] = {0};
        int duration; 
        vector<Flexoffer> individual_offers;
        AggregatedFlexOffer(int offer_id, vector<Flexoffer> &offers);
        void pretty_print();
        void schedule();
};

#endif
