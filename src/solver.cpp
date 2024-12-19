#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include "../include/solver.h"
#include "../include/aggregation.h"

ILOSTLBEGIN

//Opitimization based on NordPool for normal flexOffers
std::vector<std::vector<double>> Solver::solve(std::vector<AggregatedFlexOffer> &afos, const std::vector<double> &prices) {
    if (afos.empty()) {
        std::cerr << "No AggregatedFlexOffers to solve." << std::endl;
        return {};
    }

    IloEnv env;
    IloModel model(env);

    int A = afos.size();
    std::vector<IloNumVarArray> powerVars(A);
    IloExpr obj(env);

    // Ensure prices cover the longest duration
    int max_duration = 0;
    for (const auto &afo : afos) {
        max_duration = std::max(max_duration, afo.get_duration());
    }
    std::vector<double> adjusted_prices = prices;
    if ((int)adjusted_prices.size() < max_duration) {
        adjusted_prices.resize(max_duration, 0.0);
    }

    // Create decision variables with valid bounds
    for (int a = 0; a < A; a++) {
        int duration = afos[a].get_duration();
        powerVars[a] = IloNumVarArray(env, duration);

        auto profile = afos[a].get_aggregated_profile();
        for (int t = 0; t < duration; t++) {
            double lb = profile[t].min_power;
            double ub = profile[t].max_power;
            powerVars[a][t] = IloNumVar(env, lb, ub, ILOFLOAT);

            // Objective function: add only valid durations
            obj += adjusted_prices[t] * powerVars[a][t];
        }
    }

    model.add(IloMinimize(env, obj));
    obj.end();

    // Solve the model
    IloCplex cplex(model);
    cplex.setOut(env.getNullStream());

    if (!cplex.solve()) {
        std::cerr << "No optimal solution found." << std::endl;
        env.end();
        return {};
    }

    // Extract solution
    std::vector<std::vector<double>> solution(A);
    for (int a = 0; a < A; a++) {
        int duration = afos[a].get_duration();
        solution[a].resize(duration);
        for (int t = 0; t < duration; t++) {
            solution[a][t] = cplex.getValue(powerVars[a][t]);
        }
        afos[a].apply_schedule(solution[a]);
    }

    // Print results
    for (int a = 0; a < A; a++) {
        std::cout << "AggregatedFlexOffer " << a << " scheduled power:" << std::endl;
        for (int t = 0; t < afos[a].get_duration(); t++) {
            std::cout << "  Hour " << t << ": " << solution[a][t] << " kW" << std::endl;
        }
    }

    env.end();
    return solution;
}



//FCR for normal flexOffers
std::vector<std::vector<double>> Solver::solveCostMinimization(std::vector<AggregatedFlexOffer> &afos,
                                                               const std::vector<double> &energy_prices,
                                                               double deviation_cost) {
    if (afos.empty()) {
        std::cerr << "No AggregatedFlexOffers to solve." << std::endl;
        return {};
    }

    IloEnv env;
    try {
        IloModel model(env);

        int A = (int)afos.size();
        int max_duration = 0;
        for (const auto &afo : afos) {
            max_duration = std::max(max_duration, afo.get_duration());
        }

        // Adjust prices if not long enough
        std::vector<double> adjusted_prices = energy_prices;
        if ((int)adjusted_prices.size() < max_duration) {
            adjusted_prices.resize(max_duration, adjusted_prices.empty()? 0.0 : adjusted_prices.back());
        }

        // Decision variables:
        // powerVars[a][t]: Power allocation for AFO a at hour t
        // devVars[a][t]: Absolute deviation from baseline at hour t
        std::vector<IloNumVarArray> powerVars(A), devVars(A);

        IloExpr obj(env);

        for (int a = 0; a < A; a++) {
            int duration = afos[a].get_duration();
            auto profile = afos[a].get_aggregated_profile();

            powerVars[a] = IloNumVarArray(env, duration);
            devVars[a]   = IloNumVarArray(env, duration);

            for (int t = 0; t < duration; t++) {
                double lb = profile[t].min_power;
                double ub = profile[t].max_power;

                // Baseline: midpoint between min and max
                double baseline = 0.5 * (profile[t].min_power + profile[t].max_power);

                // Create power variable
                powerVars[a][t] = IloNumVar(env, lb, ub, ILOFLOAT);

                // Create deviation variable: devVars[a][t] >= |powerVars[a][t] - baseline|
                // devVars[a][t] must be nonnegative
                devVars[a][t] = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT);

                // Add constraints to model absolute deviation:
                // devVars[a][t] >= powerVars[a][t] - baseline
                // devVars[a][t] >= baseline - powerVars[a][t]
                model.add(devVars[a][t] >= powerVars[a][t] - baseline);
                model.add(devVars[a][t] >= baseline - powerVars[a][t]);

                // Objective contribution:
                // 1) Energy cost: price[t] * powerVars[a][t]
                // 2) Deviation cost: deviation_cost * devVars[a][t]
                obj += adjusted_prices[t] * powerVars[a][t] + deviation_cost * devVars[a][t];
            }
        }

        model.add(IloMinimize(env, obj));
        obj.end();

        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (!cplex.solve()) {
            std::cerr << "No optimal solution found." << std::endl;
            env.end();
            return {};
        }

        // Extract solution
        std::vector<std::vector<double>> solution(A);
        for (int a = 0; a < A; a++) {
            int duration = afos[a].get_duration();
            solution[a].resize(duration);
            for (int t = 0; t < duration; t++) {
                solution[a][t] = cplex.getValue(powerVars[a][t]);
            }
            afos[a].apply_schedule(solution[a]);
        }

        // Print results
        for (int a = 0; a < A; a++) {
            std::cout << "AggregatedFlexOffer " << a << " optimal schedule:" << std::endl;
            for (int t = 0; t < afos[a].get_duration(); t++) {
                std::cout << "  Hour " << t << ": " << solution[a][t] << " kW" << std::endl;
            }
        }

        env.end();
        return solution;

    } catch (IloException &e) {
        std::cerr << "CPLEX Exception: " << e.getMessage() << std::endl;
        env.end();
    } catch (std::exception &e) {
        std::cerr << "Standard Exception: " << e.what() << std::endl;
        env.end();
    }
    return {};
}
