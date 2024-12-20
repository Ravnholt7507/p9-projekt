#include <iostream>
#include <vector>

#include "../include/aggregation.h"
#include "../include/solver.h"
#include "../include/generator.h"
#include "../include/helperfunctions.h"

using namespace std;

int main() {
    string filename = "../data/spotprices.csv";
    vector<double> spotPrices = readSpotPricesFromCSV(filename);

    string evDataFile = "../data/ev_data.csv"; // Path to your EV data CSV file
    vector<Flexoffer> flexOffers = parseEVDataToFlexOffers(evDataFile);

    AggregatedFlexOffer agg1(1, Alignments::start, flexOffers);

    vector<AggregatedFlexOffer> afos = {agg1};
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

    return 0;
}
