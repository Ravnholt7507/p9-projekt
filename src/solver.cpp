#include "solver.h"
#include <ilcplex/ilocplex.h>
#include <iostream>
#include <algorithm>

ILOSTLBEGIN

std::vector<std::vector<double>> Solver::solve(std::vector<AggregatedFlexOffer> &afos,
                                              const std::vector<double> &prices,
                                              double grid_capacity) {
    IloEnv env;
    std::vector<std::vector<double>> solution;

    if (afos.empty()) {
        // No offers to solve
        return solution;
    }

    // Determine the maximum duration among all AFOs
    int max_duration = 0;
    for (const auto &afo : afos) {
        if (afo.get_duration() > max_duration) {
            max_duration = afo.get_duration();
        }
    }

    // Pad all AFOs to have the same duration
    for (auto &afo : afos) {
        if (afo.get_duration() < max_duration) {
            afo.pad_profile(max_duration);
        }
    }

    // Adjust the price vector to match the maximum duration
    std::vector<double> adjusted_prices = prices;
    if (adjusted_prices.size() < static_cast<size_t>(max_duration)) {
        // Extend the prices with the last known price or a default value
        double last_price = adjusted_prices.empty() ? 0.10 : adjusted_prices.back();
        adjusted_prices.resize(max_duration, last_price);
    } else if (adjusted_prices.size() > static_cast<size_t>(max_duration)) {
        adjusted_prices.resize(max_duration);
    }

    try {
        IloModel model(env);

        // Number of AFOs and Time Horizon
        int A = afos.size();
        int T = max_duration;

        // Decision variables: power[a][t]
        std::vector<IloNumVarArray> powerVars(A, IloNumVarArray(env, T));

        for (int a = 0; a < A; a++) {
            auto profile = afos[a].get_aggregated_profile();
            for (int t = 0; t < T; t++) {
                double lb = profile[t].min_power;
                double ub = profile[t].max_power;
                // If both min and max are 0, fix the variable to 0
                if (ub <= 0.0 && lb <= 0.0) {
                    powerVars[a][t] = IloNumVar(env, 0.0, 0.0, ILOFLOAT);
                } else {
                    powerVars[a][t] = IloNumVar(env, lb, ub, ILOFLOAT);
                }
            }
        }

        // Objective: minimize sum of prices[t] * power[a][t] over all AFOs and hours
        IloExpr obj(env);
        for (int a = 0; a < A; a++) {
            for (int t = 0; t < T; t++) {
                obj += adjusted_prices[t] * powerVars[a][t];
            }
        }
        model.add(IloMinimize(env, obj));
        obj.end();

        // Grid capacity constraints: sum of power across all AFOs at hour t <= grid_capacity
        for (int t = 0; t < T; t++) {
            IloExpr sumPower(env);
            for (int a = 0; a < A; a++) {
                sumPower += powerVars[a][t];
            }
            model.add(sumPower <= grid_capacity);
            sumPower.end();
        }

        // Solve the model
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); // Suppress CPLEX output

        if (!cplex.solve()) {
            std::cerr << "No optimal solution found." << std::endl;
            return solution; // Empty solution
        }

        // Extract solution
        solution.resize(A, std::vector<double>(T, 0.0));
        for (int a = 0; a < A; a++) {
            for (int t = 0; t < T; t++) {
                solution[a][t] = cplex.getValue(powerVars[a][t]);
            }
        }

    } catch (IloException &e) {
        std::cerr << "CPLEX Exception: " << e.getMessage() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception caught." << std::endl;
    }

    env.end();
    return solution;
}
