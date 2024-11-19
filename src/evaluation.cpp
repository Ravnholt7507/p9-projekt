#include <iostream>
#include <ctime>

#include "../include/aggregation.h"
using namespace std;

double calculateFlexibilityLoss(AggregatedFlexOffer offer){
    double originalTimeFlexibility = 0.0, aggregatedTimeFlexibility = 0.0;
    for (unsigned int i=0; i < offer.individual_offers.size(); i++){
        originalTimeFlexibility += difftime(offer.individual_offers[i].latest_start_time, offer.individual_offers[i].earliest_start_time);
    }
    aggregatedTimeFlexibility = difftime(offer.aggregated_earliest, offer.aggregated_earliest);
    return 1.0 - (aggregatedTimeFlexibility / originalTimeFlexibility);
}

void evaluateAggregatedFlexOffer(AggregatedFlexOffer offer) {
    double flexLoss = calculateFlexibilityLoss(offer);
    cout << " === AFO evalation ===" << "\n";
    cout << "Flexiblity loss: " << flexLoss << "\n";
}
