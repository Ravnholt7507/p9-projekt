#ifndef EVALUATION_H
#define EVALUATION_H
#include <vector>

#include "aggregation.h"
#include "flexoffer.h"

double calculateFlexibilityLoss(AggregatedFlexOffer offer);
void evaluateAggregatedFlexOffer(AggregatedFlexOffer offer);

#endif
