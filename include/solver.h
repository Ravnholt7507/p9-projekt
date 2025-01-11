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

    static vector<vector<double>> solve_tec(vector<AggregatedFlexOffer> &afos, const vector<double> &prices);

    static vector<double> DFO_Optimization(const DFO& dfo, const vector<double>& cost_per_unit);
};

#endif // SOLVER_H
