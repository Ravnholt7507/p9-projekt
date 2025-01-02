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

int main() {
    string filename = "../data/spotprices.csv";
    vector<double> spotPrices = readSpotPricesFromCSV(filename);
    string fcr_prices = "../data/FCRprices.csv";
    auto result = getFRCprices(fcr_prices);

    vector<double> fcr_up_prices = get<0>(result);
    vector<double> fcr_down_prices = get<1>(result);
    vector<double> fcr_up_volumes = get<2>(result);
    vector<double> fcr_down_volumes = get<3>(result);

    string evDataFile = "../data/ev_data.csv"; // Path to your EV data CSV file
    vector<variant<Flexoffer, Tec_flexoffer>> ParsedOffers = parseEVDataToFlexOffers(evDataFile, 0);
    vector<Flexoffer> flexOffers;

    // AggregatedFlexOffer agg1(0, Alignments::start, flexOffers);
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

    vector<AggregatedFlexOffer> afos = nToMAggregation(flexOffers, est_threshold, lst_threshold, max_group_size, 0);

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
    
    //auto [powerVars, upVars, downVars, totalRevenue] = Solver::solveFCRRevenueMaximization(afos, fcr_up_prices, fcr_down_prices, spotPrices);
    //dumpFCRDataToCSV(powerVars, upVars, downVars, totalRevenue, "../data/FCR_Solution.csv");

    //prepareAndDumpMetrics(spotPrices, afos, "../data/metrics.csv", "../visuals/plot_metrics.py");

    dumpSolverAndDisaggResults(afos, spotPrices, "../data/aggregator_solutions.csv", "../data/disaggregated_flexoffers.csv");

    return 0;
}
