#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include "../include/solver.h"
#include "../include/aggregation.h"

using namespace std;

static int hourOfDay(time_t t) {
    struct tm* tmInfo = localtime(&t);
    return tmInfo->tm_hour;
}

ILOSTLBEGIN

//Opitimization based on NordPool for normal flexOffers
vector<vector<double>> Solver::solve(vector<AggregatedFlexOffer> &afos, const vector<double> &prices) {
    if (afos.empty()) {
        cerr << "No AggregatedFlexOffers to solve." << endl;
        return {};
    }

    IloEnv env;
    IloModel model(env);

    int A = afos.size();
    vector<IloNumVarArray> powerVars(A);
    IloExpr obj(env);

    // Ensure prices cover the longest duration
    int max_duration = 0;
    for (const auto &afo : afos) {
        max_duration = max(max_duration, afo.get_duration());
    }
    vector<double> adjusted_prices = prices;
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
            obj += -1 * adjusted_prices[t] * powerVars[a][t];
        }
    }

    model.add(IloMaximize(env, obj));
    obj.end();

    // Solve the model
    IloCplex cplex(model);
    cplex.setOut(env.getNullStream());

    if (!cplex.solve()) {
        cerr << "No optimal solution found." << endl;
        env.end();
        return {};
    }

    // Extract solution
    vector<vector<double>> solution(A);
    for (int a = 0; a < A; a++) {
        int duration = afos[a].get_duration();
        solution[a].resize(duration);
        for (int t = 0; t < duration; t++) {
            solution[a][t] = cplex.getValue(powerVars[a][t]);
        }
        afos[a].apply_schedule(solution[a]);
        int firstNonZero = -1;
        for (int t = 0; t < duration; t++) {
            if (solution[a][t] > 0) {
                firstNonZero = t;
                break;
            }
        }
        if (firstNonZero > 0) {
            time_t scheduled_start = afos[a].get_aggregated_earliest() + (firstNonZero * 3600);
            afos[a].set_aggregated_scheduled_start_time(scheduled_start);
        } else {
            afos[a].set_aggregated_scheduled_start_time(afos[a].get_aggregated_earliest());
        }
    }

    // Print results
    for (int a = 0; a < A; a++) {
        cout << "AggregatedFlexOffer " << a << " scheduled power:" << endl;
        for (int t = 0; t < afos[a].get_duration(); t++) {
            cout << "  Hour " << t << ": " << solution[a][t] << " kW" << endl;
        }
    }

    env.end();
    return solution;
}

tuple<vector<vector<double>>, vector<vector<double>>, vector<vector<double>>, double>
Solver::solveFCRRevenueMaximization(
    vector<AggregatedFlexOffer> &afos,
    const vector<double> &up_prices,
    const vector<double> &down_prices,
    const vector<double> &energy_cost)
{
    if (afos.empty()) {
        cerr << "No AggregatedFlexOffers to solve." << endl;
        return {};
    }

    IloEnv env;
    try {
        IloModel model(env);

        // Number of aggregated flex-offers (AFOs)
        int A = static_cast<int>(afos.size());

        // Find the longest duration among all AFOs
        int max_duration = 0;
        for (const auto &afo : afos) {
            max_duration = max(max_duration, afo.get_duration());
        }

        // Create variables
        vector<IloNumVarArray> powerVars(A);
        vector<IloNumVarArray> upVars(A), downVars(A);

        // We'll build the objective expression here:
        IloExpr obj(env);

        //----------------------------------------------------------------------
        // For each Aggregated FlexOffer (AFO)
        //----------------------------------------------------------------------
        for (int a = 0; a < A; a++) {
            int duration = afos[a].get_duration();
            auto profile = afos[a].get_aggregated_profile();

            powerVars[a] = IloNumVarArray(env, duration);
            upVars[a]    = IloNumVarArray(env, duration);
            downVars[a]  = IloNumVarArray(env, duration);

            for (int t = 0; t < duration; t++) {
                // Minimum/maximum power in hour t
                double lb = profile[t].min_power;  // e.g. 0.0 kW
                double ub = profile[t].max_power;  // e.g. 5.0 kW

                powerVars[a][t] = IloNumVar(env, lb, ub, ILOFLOAT);
                upVars[a][t]    = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT);
                downVars[a][t]  = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT);

                // Constraint: up + down <= total power available
                model.add(upVars[a][t] + downVars[a][t] <= powerVars[a][t]);

                //------------------------------------------------------------------
                // Net Benefit = FCR Revenue - Cost of using power
                //------------------------------------------------------------------
                // Revenue from up/down capacity:
                //     up_prices[t] * upVars[a][t] + down_prices[t] * downVars[a][t]
                // Cost of using the (powerVars[a][t]) units of power:
                //     energy_cost[t] * powerVars[a][t]
                // => net contribution for time t:
                obj += (up_prices[t]   * upVars[a][t])
                     + (down_prices[t] * downVars[a][t])
                     - (energy_cost[t] * powerVars[a][t]);
            }
        }

        // Now we maximize the sum of revenues minus costs
        model.add(IloMaximize(env, obj));
        obj.end();

        // ---------------------------------------------------------------------
        // Solve the model
        // ---------------------------------------------------------------------
        IloCplex cplex(model);
        // If you want debug output, comment out the next line
        cplex.setOut(env.getNullStream());

        if (!cplex.solve()) {
            cerr << "No optimal solution found." << endl;
            env.end();
            return {};
        }

        // ---------------------------------------------------------------------
        // Extract the solution
        // ---------------------------------------------------------------------
        vector<vector<double>> solution(A), up_solution(A), down_solution(A);
        for (int a = 0; a < A; a++) {
            int duration = afos[a].get_duration();
            solution[a].resize(duration);
            up_solution[a].resize(duration);
            down_solution[a].resize(duration);

            for (int t = 0; t < duration; t++) {
                solution[a][t]     = cplex.getValue(powerVars[a][t]);
                up_solution[a][t]  = cplex.getValue(upVars[a][t]);
                down_solution[a][t]= cplex.getValue(downVars[a][t]);
            }
            // Optionally apply the new power schedule to AFO 'a'
            afos[a].apply_schedule(solution[a]);
        }

        // Retrieve the final objective value (total net benefit)
        double totalNetBenefit = cplex.getObjValue();

        env.end();

        // Return (power schedule, up schedule, down schedule, net benefit)
        return {solution, up_solution, down_solution, totalNetBenefit};

    } catch (IloException &e) {
        cerr << "CPLEX Exception: " << e.getMessage() << endl;
        env.end();
    } catch (exception &e) {
        cerr << "Standard Exception: " << e.what() << endl;
        env.end();
    }

    return {};
}


tuple<vector<vector<double>>, double>
solveVolumeReductionMultiple(
    vector<AggregatedFlexOffer> &allAFOs,
    const vector<vector<double>> &allOrigVolumes,
    const vector<vector<double>> &allEsch
)
{
    // Check consistent sizes
    int N = (int)allAFOs.size();
    if (N == 0 ||
        (int)allOrigVolumes.size() != N ||
        (int)allEsch.size()         != N )
    {
        cerr << "Mismatch in number of AFOs vs. input data.\n";
        return {};
    }

    IloEnv env;
    try {
        IloModel model(env);

        vector<IloNumVarArray>   vbarVars   (N);
        vector<IloNumVarArray>   vbarPlus   (N);
        vector<IloNumVarArray>   vbarMinus  (N);

        IloExpr obj(env);

        double sumAbsAllVud = 0.0;

        // Loop over each AFO i
        for (int i = 0; i < N; i++)
        {
            // 1) Get the duration T_i for the i-th aggregator
            int T_i = allAFOs[i].get_duration();

            // Basic checks
            if ((int)allOrigVolumes[i].size() < T_i ||
                (int)allEsch[i].size() < T_i)
            {
                cerr << "AFO " << i << ": mismatch in time slices or e_sch.\n";
                continue;
            }

            // 2) Sum up |v_udt^i| to add to the constant part of the objective
            for (int t = 0; t < T_i; t++) {
                sumAbsAllVud += fabs(allOrigVolumes[i][t]);
            }

            // 3) Create the variables for this AFO in CPLEX
            vbarVars[i]  = IloNumVarArray(env, T_i, -IloInfinity, IloInfinity, ILOFLOAT);
            vbarPlus[i]  = IloNumVarArray(env, T_i, 0.0, IloInfinity, ILOFLOAT);
            vbarMinus[i] = IloNumVarArray(env, T_i, 0.0, IloInfinity, ILOFLOAT);

            // 4) We'll retrieve the aggregator’s min/max from each timeslice
            auto profile = allAFOs[i].get_aggregated_profile();
            
            // Add constraints for each time slot
            for (int t = 0; t < T_i; t++) {
                double lb = profile[t].min_power;
                double ub = profile[t].max_power;

                model.add( vbarVars[i][t] >= lb );
                model.add( vbarVars[i][t] <= ub );
                model.add( vbarVars[i][t] == vbarPlus[i][t] - vbarMinus[i][t] );

                double eschVal = allEsch[i][t];
                model.add( vbarPlus[i][t] + vbarMinus[i][t] >= eschVal );

            }
            for (int t = 0; t < T_i; t++) {
                obj -= (vbarPlus[i][t] + vbarMinus[i][t]);
            }
        }

        IloObjective objective = IloMaximize(env, sumAbsAllVud + obj);
        model.add(objective);
        obj.end();

        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); // quiet the solver

        if (!cplex.solve()) {
            cerr << "CPLEX: no optimal solution found.\n";
            env.end();
            return {};
        }

        double totalObj = cplex.getObjValue();

        // ------------------------------------------------------------
        // Extract the solution 
        // ------------------------------------------------------------
        // We’ll store them in a 2D array: solutions[i][t]
        vector<vector<double>> solutions(N);

        for (int i = 0; i < N; i++) {
            int T_i = allAFOs[i].get_duration();
            solutions[i].resize(T_i);

            for (int t = 0; t < T_i; t++) {
                solutions[i][t] = cplex.getValue(vbarVars[i][t]);
            }

            // Optionally, apply the schedule to the aggregator 
            allAFOs[i].apply_schedule(solutions[i]);
        }

        env.end();

        // Return the solutions for each AFO + total objective
        return make_tuple(solutions, totalObj);

    } catch (IloException &e) {
        cerr << "CPLEX exception: " << e.getMessage() << endl;
        env.end();
    } catch (exception &e) {
        cerr << "std::exception: " << e.what() << endl;
        env.end();
    }

    // Return empty in case of errors
    return {};
}



void Solver::DFO_Optimization(const DFO& dfo, const vector<double>& cost_per_unit) {
    IloEnv env;
    try {
        IloModel model(env);
        IloNumVarArray energy(env, dfo.polygons.size(), 0.0, IloInfinity, ILOFLOAT);
        IloExpr totalCost(env);

        // Objective: Minimize total cost
        for (size_t t = 0; t < dfo.polygons.size(); ++t) {
            totalCost += cost_per_unit[t] * energy[t];
        }
        model.add(IloMinimize(env, totalCost));
        totalCost.end();

        // Constraints: Ensure scheduling adheres to dependency polygons
        for (size_t t = 0; t < dfo.polygons.size(); ++t) {
            const auto& polygon = dfo.polygons[t];
            IloExpr dependency(env);

            for (size_t i = 0; i < t; ++i) {
                dependency += energy[i];
            }

            for (const auto& point : polygon.points) {
                if (point.x >= 0 && point.y >= 0) {
                    model.add(dependency <= point.x);
                    model.add(energy[t] >= point.y);
                    model.add(energy[t] <= point.x);
                }
            }
            dependency.end();
        }

        // Solve the model
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); // Suppress output for performance
        if (cplex.solve()) {
            cout << "Optimal solution found!" << endl;
            for (size_t t = 0; t < dfo.polygons.size(); ++t) {
                cout << "Energy at time " << t + 1 << ": " << cplex.getValue(energy[t]) << endl;
            }
        } else {
            cerr << "Failed to find optimal solution." << endl;
        }

    } catch (const IloException& e) {
        cerr << "Error: " << e.getMessage() << endl;
    } catch (...) {
        cerr << "Unknown error occurred." << endl;
    }
    env.end();
}


vector<vector<double>> Solver::solve_tec(vector<AggregatedFlexOffer> &afos, const vector<double> &prices) {
    if (afos.empty()) {
        cerr << "No AggregatedFlexOffers to solve." << endl;
        return {};
    }

    IloEnv env;
    IloModel model(env);

    int A = afos.size();
    vector<IloNumVarArray> powerVars(A);
    IloExpr obj(env);

    // Ensure prices cover the longest duration
    int max_duration = 0;
    for (const auto &afo : afos) {
        max_duration = max(max_duration, afo.get_duration());
    }
    vector<double> adjusted_prices = prices;
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
        IloExpr total_energy(env);
        for (int t = 0; t < duration; t++) {
            total_energy += powerVars[a][t];
        }
        model.add(total_energy >= afos[a].get_min_overall());
        model.add(total_energy <= afos[a].get_max_overall());
        total_energy.end();
    }

    model.add(IloMinimize(env, obj));
    obj.end();

    // Solve the model
    IloCplex cplex(model);
    cplex.setOut(env.getNullStream());

    if (!cplex.solve()) {
        cerr << "No optimal solution found." << endl;
        env.end();
        return {};
    }

    // Extract solution
    vector<vector<double>> solution(A);
    for (int a = 0; a < A; a++) {
        int duration = afos[a].get_duration();
        solution[a].resize(duration);
        for (int t = 0; t < duration; t++) {
            solution[a][t] = cplex.getValue(powerVars[a][t]);
        }
        afos[a].apply_schedule(solution[a]);
        int firstNonZero = -1;
        for (int t = 0; t < duration; t++) {
            if (solution[a][t] > 0) {
                firstNonZero = t;
                break;
            }
        }
        if (firstNonZero > 0) {
            time_t scheduled_start = afos[a].get_aggregated_earliest() + (firstNonZero * 3600);
            afos[a].set_aggregated_scheduled_start_time(scheduled_start);
        } else {
            afos[a].set_aggregated_scheduled_start_time(afos[a].get_aggregated_earliest());
        }
    }


    for (int a = 0; a < A; a++) {
        cout << "AggregatedFlexOffer " << a << " scheduled power:" << endl;
        cout << "Scheduled start time " << hourOfDay(afos[a].get_aggregated_scheduled_start_time()) << endl;
        for (int t = 0; t < afos[a].get_duration(); t++) {
            cout << "  Hour " << t << ": " << solution[a][t] << " kW" << endl;
        }
    }

    env.end();
    return solution;
}
