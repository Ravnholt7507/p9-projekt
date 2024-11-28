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

    grid.prettyprint();
    gh.prettyprint();

    auto mbr = calculateMBR(flexOffers);

    cout << "Min values: [" << mbr.first[0] << ", " << mbr.first[1] << "]\n";
    cout << "Max values: [" << mbr.second[0] << ", " << mbr.second[1] << "]\n";
    vector<int> thresholds = {2, 2};

    cout << doesMBRExceedThreshold(mbr, thresholds);
    



    // AggregatedFlexOffer obj(0, flexOffers);
    // obj.pretty_print();
    // ScheduledFlexOffer SFO(obj);
    // SFO.print_schedule();
    // SFO.n_to_1_disaggregation(obj.individual_offers, obj);

    return 0;
}
