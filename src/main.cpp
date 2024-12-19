#include <iostream>
#include <vector>
#include "flexoffer.h"
#include "aggregation.h"
#include "solver.h"
#include "generator.h"


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

    // 4. Solve the optimization problem
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





// #include "../include/flexoffer.h"
// #include "../include/generator.h"
// #include "../include/group.h"
// #include "economic.h"
// #include "../include/helperfunctions.h"
// #include <iostream>
// #include <vector>
// #include <ctime>
// #include "flexoffer.h"
// #include "solver.h"

// using namespace std;

// int main() {

//     // Define hourly prices (€/kWh)
//     vector<double> prices = {
//         0.05, 0.04, 0.04, 0.03, 0.03, 0.03,
//         0.04, 0.05, 0.06, 0.08, 0.10, 0.12,
//         0.14, 0.13, 0.11, 0.09, 0.07, 0.06,
//         0.05, 0.05, 0.04, 0.04, 0.05, 0.05
//     };

//     int numOffers = 10;
//     vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

//     // Optimize scheduling
//     vector<FlexofferSolution> solutions;
//     bool success = Solver::solve_optimization(flexOffers, prices, solutions);

//     // Display results
//     if (success) {
//         cout << "Solution found!" << endl;
//         for (const auto &sol : solutions) {
//             cout << "Flexoffer " << sol.offer_id << " start time: " << ctime(&sol.scheduled_start_time);
//             cout << "Allocated powers: ";
//             for (const auto &p : sol.scheduled_allocation) cout << p << " kW ";
//             cout << endl;
//         }
//     } else {
//         cout << "No solution found." << endl;
//     }

//     return 0;
// }










// using namespace std;

// int main() {
//     int numOffers = 1;
//     vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

//     // Create one group per Flexoffer
//     std::vector<Group> groups;
//     int groupId = 1;

//     for (const auto& fo : flexOffers) {
//         Group g(groupId++);
//         g.addFlexOffer(fo);
//         groups.push_back(g);
//     }

//     int est_threshold = 2;
//     int lst_threshold = 2; 
//     int max_group_size = 3;

//     // Perform bottom-up hierarchical clustering with max_group_size constraint
//     clusterGroup(groups, est_threshold, lst_threshold, max_group_size);

//     // Print final groups
//     for (auto& g : groups) {
//         g.printAggregatedOffer();
//     }

//     time_t now = time(NULL);
//     vector<TimeSlice> profile = {
//         {1.0, 4.0}, // Hour 0
//         {0.5, 3.0}, // Hour 1
//         {2.0, 5.0}, // Hour 2
//         {1.0, 2.0}  // Hour 3
//     };
//     Flexoffer fo(1, now, now + 3600, now + 14400, profile, 3);

//     // Mock price vector for the 4 hours
//     vector<double> price_per_hour = {0.30, 0.45, 0.25, 0.50}; // currency/kWh

//     // Compute target energy and schedule
//     double target_energy = compute_target_energy(fo);
//     vector<double> schedule = compute_optimal_schedule(fo, price_per_hour, target_energy);
//     fo.set_scheduled_allocation(schedule);

//     // Compute cost
//     double total_cost = compute_total_cost(fo, schedule, price_per_hour);

//     // Print results
//     fo.print_flexoffer();
//     cout << "Chosen schedule (kW per hour):" << endl;
//     for (int i = 0; i < fo.get_duration(); i++) {
//         cout << "Hour " << i << ": " 
//              << schedule[i] << " kW (Price: " << price_per_hour[i] << " currency/kWh)" << endl;
//     }
//     cout << fixed << setprecision(2);
//     cout << "Total Cost: " << total_cost << " currency" << endl;

//     return 0;
// }
