#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
#include "aggregation.h" // For AggregatedFlexOffer

class Solver {
public:
    // Solve using CPLEX to minimize sum of all (price[t]*power[t]) for all AFOs
    // subject to:
    //  min_power[t] <= power[t] <= max_power[t] for each AFO and hour
    // and sum of power across all AFOs per hour <= grid_capacity
    //
    // Returns a vector of allocations per AFO. allocations[i][t] = power for AFO i at hour t.
    static std::vector<std::vector<double>> solve(std::vector<AggregatedFlexOffer> &afos,
                                                 const std::vector<double> &prices,
                                                 double grid_capacity);
};

#endif
