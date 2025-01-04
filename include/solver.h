#ifndef SOLVER_H
#define SOLVER_H

#include "aggregation.h"
#include "DFO.h"
#include <vector>

using namespace std;

class Solver {
public:
    // Existing method
    static vector<vector<double>> solve(vector<AggregatedFlexOffer> &afos, const vector<double> &prices);

    static tuple<vector<vector<double>>, vector<vector<double>>, vector<vector<double>>, double> solveFCRRevenueMaximization(
    vector<AggregatedFlexOffer> &afos,
    const vector<double> &up_prices,
    const vector<double> &down_prices,
    const vector<double> &energy_cost);

    static tuple<vector<vector<double>>, double>
    solveVolumeReductionMultiple(
    vector<AggregatedFlexOffer> &allAFOs,
    const vector<vector<double>> &allOrigVolumes,
    const vector<vector<double>> &allEsch);

    static vector<vector<double>> solve_tec(vector<AggregatedFlexOffer> &afos, const vector<double> &prices);

    static vector<double> DFO_Optimization(const DFO& dfo, const vector<double>& cost_per_unit);
};

#endif // SOLVER_H
