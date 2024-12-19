#include <iostream>
#include <vector>

#include "../include/tec.h"
#include "../include/aggregation.h"
#include "../include/solver.h"
#include "../include/generator.h"

using namespace std;

int main() {
    int numOffers = 2;
    vector<Tec_flexoffer> flexOffers = generateMultipleTecFlexOffers(numOffers);

    for(auto& offer : flexOffers){
        offer.print_flexoffer();
    }

    vector<Tec_flexoffer> set1 = {flexOffers[0]};
    vector<Tec_flexoffer> set2 = {flexOffers[1]};

    AggregatedFlexOffer agg1(1, set1);
    AggregatedFlexOffer agg2(2, set2);

    cout << "\n=== Aggregated TEC 1 (Before Scheduling) ===\n";
    agg1.pretty_print();

    cout << "\n=== Aggregated TEC 2 (Before Scheduling) ===\n";
    agg2.pretty_print();

    int max_duration = max(agg1.get_duration(), agg2.get_duration());
    vector<double> prices(max_duration, 0.10);

    vector<AggregatedFlexOffer> afos = {agg1, agg2};
    vector<vector<double>> solution = Solver::solve(afos, prices);

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
            total_cost += sched[t] * prices[t];
        }
    }

    cout << "Total Cost (Optimized): " << total_cost << " €/kWh\n";
    cout << "=== Done ===" << endl;

    return 0;
}
