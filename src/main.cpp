#include <vector>

#include "../include/flexoffer.h"
#include "../include/generator.h"
#include "../include/grid.h"
#include "../include/group.h"
#include "../include/helperfunctions.h"


using namespace std;

int main(){
    int numOffers = 50;
    Grid grid;
    GroupHash gh(grid);

    vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

    for(int i = 0; i < numOffers; i++){
        flexOffers[i].print_flexoffer();
        gh.addFlexOffer(flexOffers[i]);
    }

    vector<int> thresholds = {2, 2};

    optimizeGroup(1, gh, thresholds, flexOffers);

    return 0;
}
