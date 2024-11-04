#include <ctime>
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>
#include "scheduled_flexoffer.h"
#include "../flexoffers/flexoffer.h"
#include "../aggregation/aggregation.h"
using namespace std;

ScheduledFlexOffer::ScheduledFlexOffer(AggregatedFlexOffer& AFO)
    : aggregated_offer(AFO){

    scheduled_profile.resize(24);
    for (int i = 0; i < 24; i++) {
        scheduled_profile[i] = (aggregated_offer.aggregated_profile[i].min_power + 
                                aggregated_offer.aggregated_profile[i].max_power) / 2.0;
    }
}


void ScheduledFlexOffer::print_schedule() {
    std::cout << "Scheduled allocation: " << std::endl;
    for (size_t i = 0; i < aggregated_offer.aggregated_profile.size(); i++) {
        std::cout << "  Hour " << i << ": Scheduled Power = " 
                  << fixed << setprecision(2) << scheduled_profile[i] << " kW" << endl;
    }
};

