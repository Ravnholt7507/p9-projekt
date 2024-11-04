#ifndef SCHEDULED_FLEX_OFFER_H
#define SCHEDULED_FLEX_OFFER_H

#include <ctime>
#include <vector>
#include "../flexoffers/flexoffer.h"
#include "../aggregation/aggregation.h"
using namespace std;


class ScheduledFlexOffer {
public:
    AggregatedFlexOffer &aggregated_offer;
    int offer_id;
    vector<double> scheduled_profile;
    ScheduledFlexOffer(AggregatedFlexOffer &AFO);
    
    void schedule();
    void print_schedule();
};

#endif // SCHEDULED_FLEX_OFFER_H
