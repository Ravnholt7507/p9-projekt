#ifndef SOLVER_H
#define SOLVER_H

#include "aggregation.h"
#include "DFO.h"
#include <vector>

class Solver {
public:
    // Existing method
    static std::vector<std::vector<double>> solve(std::vector<AggregatedFlexOffer> &afos, const std::vector<double> &prices);

    // New method for cost minimization with deviation penalties
    std::vector<std::vector<double>> solveCostMinimization(std::vector<AggregatedFlexOffer> &afos,
                                                           const std::vector<double> &energy_prices,
                                                           double deviation_cost);

    static std::vector<std::vector<double>> solve_tec(std::vector<AggregatedFlexOffer> &afos, const std::vector<double> &prices);

    static void DFO_Optimization(const DFO& dfo, const std::vector<double>& cost_per_unit);
};

#endif // SOLVER_H
