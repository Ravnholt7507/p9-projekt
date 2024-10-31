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

    Flexoffer aggregatedOffer = AggregateFlexOffers(flexOffers);
    aggregatedOffer.print_flexoffer();

    evaluateAggregatedFlexOffer(flexOffers, aggregatedOffer);
    return 0;
}

