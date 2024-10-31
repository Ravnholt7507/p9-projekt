#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include "random"
#include "vector"
#include "../flexoffers/flexoffer.h"
using namespace std;

double calculateFlexibilityLoss(vector<Flexoffer> originalOffers, Flexoffer aggregatedOffer){
    double originalTimeFlexibility = 0.0, aggregatedTimeFlexibility = 0.0;
    for (unsigned int i=0; i < originalOffers.size(); i++){
        originalTimeFlexibility += difftime(originalOffers[i].latest_start_time, originalOffers[i].earliest_start_time);
    }
    aggregatedTimeFlexibility = difftime(aggregatedOffer.latest_start_time, aggregatedOffer.earliest_start_time);
    return 1.0 - (aggregatedTimeFlexibility / originalTimeFlexibility);
}

void evaluateAggregatedFlexOffer(vector<Flexoffer> originalOffers, Flexoffer aggregatedOffer) {
    double flexLoss = calculateFlexibilityLoss(originalOffers, aggregatedOffer);
    cout << " === AFO evalation ===" << "\n";
    cout << "Flexiblity loss: " << flexLoss << "\n";
}