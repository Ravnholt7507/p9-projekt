#ifndef EVALUATION_H
#define EVALUATION_H

#include <vector>

#include "../include/flexoffer.h"
#include "../include/tec.h"
#include "../include/alignments.h"

using namespace std;

struct AggScenario {
    int aggregator_type;
    int est_threshold;
    int lst_threshold;
    int max_group_size;
    Alignments align;
    int usedOffers;
};

double computeBaselineCost(const vector<Flexoffer> &flexOffers, const vector<double> &spotPrices);
double computeBaselineCost(const vector<Tec_flexoffer> &flexOffers, const vector<double> &spotPrices);

double computeAggregatedCost(vector<Flexoffer> flexOffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, const vector<double> &spotPrices);
double computeAggregatedCost(vector<Tec_flexoffer> flexOffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, const vector<double> &spotPrices);

void runAggregationScenarios(const vector<Flexoffer> &normalOffers, const vector<Tec_flexoffer> &tecOffers,const vector<double> &spotPrices);
vector<AggScenario> generateScenarioMatrix();

#endif
