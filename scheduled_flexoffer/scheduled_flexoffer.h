// ScheduledFlexOffer.h
#ifndef SCHEDULED_FLEX_OFFER_H
#define SCHEDULED_FLEX_OFFER_H

#include "../flexoffers/flexoffer.h"
#include "../aggregation/aggregation.h"
#include <vector>
#include <ctime>

class ScheduledFlexOffer {
public:
    // Constructor
    ScheduledFlexOffer(AggregatedFlexOffer offer);
    int id;
    time_t scheduled_start_time;
    time_t scheduled_end_time;
    std::vector<TimeSlice> scheduled_profile;


};

#endif
