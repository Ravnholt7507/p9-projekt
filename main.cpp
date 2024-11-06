#include <vector>
#include "flexoffers/flexoffer.h"
#include "aggregation/aggregation.h"
#include "scheduled_flexoffer/scheduled_flexoffer.h"
using namespace std;

int main(){
    int numOffers = 5;

    vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

    for(int i = 0; i < numOffers; i++){
        flexOffers[i].print_flexoffer();
    }

    AggregatedFlexOffer obj(0, flexOffers);
    obj.pretty_print();
    ScheduledFlexOffer SFO(obj);
    SFO.print_schedule();
    SFO.n_to_1_disaggregation(obj.individual_offers, obj);

    return 0;
}

