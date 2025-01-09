#ifndef SOLVER_H
#define SOLVER_H

#include "aggregation.h"
#include "DFO.h"
#include <vector>

using namespace std;

struct SolverResult {
    vector<double> finalSchedule;
    vector<double> newDeviation;
    double totalReduction;
};


class Solver {
public:
    // Existing method
    static vector<vector<double>> solve(vector<AggregatedFlexOffer> &afos, const vector<double> &prices);

    static vector<vector<double>> solve_tec(vector<AggregatedFlexOffer> &afos, const vector<double> &prices);

    static vector<double> DFO_Optimization(const DFO& dfo, const vector<double>& cost_per_unit);

    static SolverResult solveFCR(AggregatedFlexOffer &afo, const vector<double> &mismatch);

    static SolverResult solveFCR_tec(AggregatedFlexOffer &afo, const std::vector<double> &mismatch);
};

#endif // SOLVER_H
