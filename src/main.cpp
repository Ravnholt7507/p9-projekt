#include <iostream>
#include <vector>
#include <variant> 

#include "../include/group.h"
#include "../include/aggregation.h"
#include "../include/solver.h"
#include "../include/generator.h"
#include "../include/helperfunctions.h"
#include "../include/tec.h"

using namespace std;




vector<AggregatedFlexOffer> nToMAggregation(const std::vector<Flexoffer> &allFlexoffers, 
                                            int est_threshold, 
                                            int lst_threshold, 
                                            int max_group_size, 
                                            int startGroupId=1)
{
    vector<Group> groups;
    int groupId = startGroupId;
    for (const auto &fo : allFlexoffers) {
        Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    clusterGroup(groups, est_threshold, lst_threshold, max_group_size);

    std::vector<AggregatedFlexOffer> finalAggregates;
    finalAggregates.reserve(groups.size());
    for (auto &g : groups) {
        // Now each group has at least one Flexoffer, so aggregator won't throw
        finalAggregates.push_back(g.createAggregatedOffer());
    }

    return finalAggregates;
}


int main() {
    string filename = "../data/spotprices.csv";
    vector<double> spotPrices = readSpotPricesFromCSV(filename);

    string evDataFile = "../data/ev_data.csv"; // Path to your EV data CSV file
    vector<variant<Flexoffer, Tec_flexoffer>> ParsedOffers = parseEVDataToFlexOffers(evDataFile, 0);
    vector<Flexoffer> flexOffers;

    int est_threshold  = 2;
    int lst_threshold  = 2;
    int max_group_size = 3;

    if(holds_alternative<Flexoffer>(ParsedOffers.front())) {
        for (auto offer : ParsedOffers){
            flexOffers.push_back(get<Flexoffer>(offer));
        }
    } else {
        for (auto offer : ParsedOffers){
            flexOffers.push_back(get<Tec_flexoffer>(offer));
        }
    }

    for (auto fo : flexOffers) {
        fo.print_flexoffer();
    }

    vector<AggregatedFlexOffer> afos = nToMAggregation(flexOffers, est_threshold, lst_threshold, max_group_size);


    // AggregatedFlexOffer agg1(1, flexOffers);

    // vector<AggregatedFlexOffer> afos = {agg1};
    vector<vector<double>> solution = Solver::solve(afos, spotPrices);

    cout << "\n=== Results ===\n";
    cout << "--- Optimized ---\n";
    for (auto &afo : afos) {
        afo.pretty_print();
    }

    // Compute total cost
    double total_cost = 0.0;
    for (size_t a = 0; a < afos.size(); a++) {
        const auto &sched = afos[a].get_scheduled_allocation();
        for (size_t t = 0; t < sched.size(); t++) {
            total_cost += sched[t] * spotPrices[t];
        }
    }

    cout << "Total Cost (Optimized): " << total_cost << " €/kWh\n";
    cout << "=== Done ===" << endl;

    vector<Flexoffer> dis_FOs = afos[0].disaggregate_to_flexoffers();

    cout << "after disaggregation";
    for (auto &fo : dis_FOs) {
        fo.print_flexoffer();
    }

    return 0;
}
