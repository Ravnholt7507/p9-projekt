#include <iostream>
#include <vector>

#include "../include/flexoffer.h"
#include "../include/aggregation.h"
#include "../include/solver.h"
#include "../include/generator.h"

int main() {
    int numOffers = 2;
    vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

    std::vector<Flexoffer> set1 = {flexOffers[0]};
    std::vector<Flexoffer> set2 = {flexOffers[1]};

    AggregatedFlexOffer agg1(1, set1);
    AggregatedFlexOffer agg2(2, set2);

    std::cout << "\n=== Aggregated FlexOffer 1 (Before Scheduling) ===\n";
    agg1.pretty_print();

    std::cout << "\n=== Aggregated FlexOffer 2 (Before Scheduling) ===\n";
    agg2.pretty_print();

    int max_duration = std::max(agg1.get_duration(), agg2.get_duration());
    std::vector<double> prices(max_duration, 0.10);

    std::vector<AggregatedFlexOffer> afos = {agg1, agg2};
    std::vector<std::vector<double>> solution = Solver::solve(afos, prices);

    std::cout << "\n=== Results ===\n";
    std::cout << "--- Optimized ---\n";
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

    std::cout << "Total Cost (Optimized): " << total_cost << " €/kWh\n";
    std::cout << "=== Done ===" << std::endl;

    return 0;
}
