#ifndef SOLVER_H
#define SOLVER_H

#include "aggregation.h"
#include <vector>

class Solver {
public:
    // Existing method
    std::vector<std::vector<double>> solve(std::vector<AggregatedFlexOffer> &afos, const std::vector<double> &prices);

    // New method for cost minimization with deviation penalties
    std::vector<std::vector<double>> solveCostMinimization(std::vector<AggregatedFlexOffer> &afos,
                                                           const std::vector<double> &energy_prices,
                                                           double deviation_cost);

};

#endif // SOLVER_H
