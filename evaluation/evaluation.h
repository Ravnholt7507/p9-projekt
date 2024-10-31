#ifndef EVALUATION_H
#define EVALUATION_H
#include <vector>
#include "../flexoffers/flexoffer.h"

double calculateFlexibilityLoss(vector<Flexoffer> originalOffers, Flexoffer aggregatedOffer);
void evaluateAggregatedFlexOffer(vector<Flexoffer> originalOffers, Flexoffer aggregatedOffer);

#endif