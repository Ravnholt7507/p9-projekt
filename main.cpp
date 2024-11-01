#include <vector>
#include "flexoffers/flexoffer.h"
#include "aggregation/aggregation.h"
using namespace std;

int main(){
    int numOffers = 1;
    vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

    for(int i = 0; i < numOffers; i++){
        flexOffers[i].print_flexoffer();
    }

    AggregatedFlexOffer obj(0, flexOffers);
    obj.pretty_print();
    
    return 0;
}

