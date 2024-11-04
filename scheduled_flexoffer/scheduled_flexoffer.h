#ifndef SCHEDULED_FLEX_OFFER_H
#define SCHEDULED_FLEX_OFFER_H

#include <ctime>
#include <vector>
#include "flexoffers/flexoffer.h"

class ScheduledFlexOffer {
public:
    int offer_id;
    time_t scheduled_start_time;
    std::vector<TimeSlice> scheduled_profile;
    ScheduledFlexOffer(int id, time_t start_time, const std::vector<TimeSlice>& profile);

    void printScheduledDetails() const;
    void execute() const;

};

#endif // SCHEDULED_FLEX_OFFER_H
