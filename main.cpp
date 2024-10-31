#include <vector>
#include "flexoffers/flexoffer.h"
using namespace std;

int main(){
    int numOffers = 5;
    vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

    for(int i = 0; i < numOffers; i++){
        flexOffers[i].print_flexoffer();
    }

    return 0;
}

