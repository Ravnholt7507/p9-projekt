#ifndef EVALUATION_H
#define EVALUATION_H
#include <vector>
#include "../aggregation/aggregation.h"
#include "../flexoffers/flexoffer.h"

double calculateFlexibilityLoss(AggregatedFlexOffer offer);
void evaluateAggregatedFlexOffer(AggregatedFlexOffer offer);

#endif
