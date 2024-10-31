#include <vector>
#include "flexoffers/flexoffer.h"
#include "aggregation/aggregation.h"
#include "evaluation/evaluation.h"
using namespace std;

int main(){
    int numOffers = 5;
    vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

    for(int i = 0; i < numOffers; i++){
        flexOffers[i].print_flexoffer();
    }

    AggregatedFlexOffer obj(0, flexOffers);
    obj.pretty_print();
    Flexoffer aggregatedOffer(1, obj.aggregated_earliest, obj.aggregated_earliest, obj.aggregated_end_time, obj.aggregated_profile, obj.duration);
    evaluateAggregatedFlexOffer(obj.individual_offers, aggregatedOffer);
    return 0;
}

