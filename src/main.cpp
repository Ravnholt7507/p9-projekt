#include <vector>

#include "../include/flexoffer.h"
#include "../include/aggregation.h"
#include "../include/scheduled_flexoffer.h"
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


    // AggregatedFlexOffer obj(0, flexOffers);
    // obj.pretty_print();
    // ScheduledFlexOffer SFO(obj);
    // SFO.print_schedule();
    // SFO.n_to_1_disaggregation(obj.individual_offers, obj);

    // auto mbr = calculateMBR(flexOffers);

    // cout << "Min values: [" << mbr.first[0] << ", " << mbr.first[1] << "]\n";
    // cout << "Max values: [" << mbr.second[0] << ", " << mbr.second[1] << "]\n";

    // cout << doesMBRExceedThreshold(mbr, thresholds);
