#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include "../include/solver.h"
#include "../include/DFO.h"
#include "../include/aggregation.h"

using namespace std;

ILOSTLBEGIN

//Opitimization based on NordPool for normal flexOffers
vector<vector<double>> Solver::solve(vector<AggregatedFlexOffer> &afos, const vector<double> &prices){

    if (afos.empty()) {
        cerr << "[DEBUG] No AggregatedFlexOffers => returning empty.\n";
        return {};
    }

    IloEnv env;
    IloModel model(env);

    int A = (int)afos.size();
    vector<IloNumVarArray> powerVars(A);
    IloExpr obj(env);

    // 3) Build decision variables
    for (int a = 0; a < A; a++) {
        int duration = afos[a].get_duration();
        powerVars[a] = IloNumVarArray(env, duration);
        auto profile = afos[a].get_aggregated_profile();

        for (int t = 0; t < duration; t++) {
            double lb = profile[t].min_power;
            double ub = profile[t].max_power;

            // Debug
            //cout << "[DEBUG] AFO=" << a << " t=" << t << " lb=" << lb 
            //     << " ub=" << ub << " price=" << adjusted_prices[t] << endl;

            powerVars[a][t] = IloNumVar(env, lb, ub, ILOFLOAT);
            obj += 1.0 * prices[t + afos[a].get_aggregated_earliest_hour()] * powerVars[a][t];
        }
    }

    model.add(IloMinimize(env, obj));
    obj.end();

    // 4) Solve
    IloCplex cplex(model);
    cplex.setOut(env.getNullStream());
    bool ok = cplex.solve();
    if (!ok) {
        cerr << "[ERROR] No optimal solution found => cplex status="
             << cplex.getStatus() << endl;
        env.end();
        return {};
    }

    // 5) Extract solution
    vector<vector<double>> solution(A);
    for (int a = 0; a < A; a++) {
        int duration = afos[a].get_duration();
        solution[a].resize(duration);
        for (int t = 0; t < duration; t++) {
            // If cplex throws NotExtracted => check if you see the var in the model
            double val = cplex.getValue(powerVars[a][t]);
            solution[a][t] = val;
        }
        afos[a].apply_schedule(solution[a]);

        // Shift aggregator's earliest if needed
        int firstNonZero = -1;
        for (int t = 0; t < duration; t++) {
            if (solution[a][t] > 0.0) {
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
        cout << "[DEBUG] AFO " << a << " => final schedule => duration=" << duration
             << endl;
    }

    env.end();
    return solution;
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

    // Create decision variables with valid bounds
    for (int a = 0; a < A; a++) {
        int duration = afos[a].get_duration();
        powerVars[a] = IloNumVarArray(env, duration);
        cout << "Duration: " << duration << "\n"; 

        auto profile = afos[a].get_aggregated_profile();
        for (int t = 0; t < duration; t++) {
            double lb = profile[t].min_power;
            double ub = profile[t].max_power;
            powerVars[a][t] = IloNumVar(env, lb, ub, ILOFLOAT);

            // Objective function: add only valid durations
            obj += prices[t + afos[a].get_aggregated_earliest_hour()] * powerVars[a][t];
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

    env.end();
    return solution;
}



void add_linear_interpolation_constraints(
    IloModel& model,
    IloEnv& env,
    IloNumVar energy,
    IloNumExpr cumulative_dependency,
    const vector<Point>& points
) {
    for (size_t k = 1; k + 1 < points.size(); k += 2) {
        const auto& prev_point_min = points[k - 1];
        const auto& prev_point_max = points[k];
        const auto& next_point_min = points[k + 1];
        const auto& next_point_max = points[k + 2];

        // Check if cumulative_dependency is within the relevant range
        IloConstraint lower_bound = (cumulative_dependency >= prev_point_min.x);
        IloConstraint upper_bound = (cumulative_dependency <= next_point_min.x);

        // Add constraints for the valid range and break after finding the relevant range
        if (prev_point_min.x <= next_point_min.x) {
            model.add(lower_bound);
            model.add(upper_bound);

            // Linear interpolation for min and max energy
            IloNumExpr min_energy(env);
            IloNumExpr max_energy(env);

            min_energy = prev_point_min.y +
                         ((next_point_min.y - prev_point_min.y) /
                          (next_point_min.x - prev_point_min.x)) *
                         (cumulative_dependency - prev_point_min.x);

            max_energy = prev_point_max.y +
                         ((next_point_max.y - prev_point_max.y) /
                          (next_point_max.x - prev_point_max.x)) *
                         (cumulative_dependency - prev_point_max.x);

            // Add constraints for energy
            model.add(energy >= min_energy);
            model.add(energy <= max_energy);

            // End expressions to free memory
            min_energy.end();
            max_energy.end();

            break; // Stop after finding the relevant pair
        }
    }
}

vector<double> Solver::DFO_Optimization(const DFO& dfo, const vector<double>& cost_per_unit) {
    IloEnv env;
    vector<double> DFO_Schedule;
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
        IloNumExpr cumulative_dependency(env); // Running sum of energy variables

        for (size_t t = 0; t < dfo.polygons.size(); ++t) {
            const auto& polygon = dfo.polygons[t];

            // Add interpolation constraints for the current dependency polygon
            add_linear_interpolation_constraints(model, env, energy[t], cumulative_dependency, polygon.points);

            // Update cumulative dependency for the next timestep
            cumulative_dependency += energy[t];
        }
        cumulative_dependency.end();

        // Solve the model
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); // Suppress output for performance
        if (cplex.solve()) {
            for (size_t t = 0; t < dfo.polygons.size(); ++t) {
                double value = cplex.getValue(energy[t]);
                DFO_Schedule.push_back(value);
            //    cout << "Energy at time " << t + 1 << ": " << value << endl;
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
    return DFO_Schedule;
}

SolverResult Solver::solveFCR(AggregatedFlexOffer &afo, const vector<double> &mismatch){
    SolverResult result;
    result.totalReduction = 0.0;

    int T = afo.get_duration();
    IloEnv env;
    IloModel model(env);

    IloNumVarArray p(env, T);
    auto profile = afo.get_aggregated_profile();
    for (int t = 0; t < T; t++) {
        double lb = profile[t].min_power;
        double ub = profile[t].max_power;
        p[t] = IloNumVar(env, lb, ub, ILOFLOAT);
    }

    IloNumVarArray y(env, T);
    for (int t = 0; t < T; t++) {
        y[t] = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT);
    }

    for (int t = 0; t < T; t++) {
        double v = mismatch[t + afo.get_aggregated_earliest_hour()];
        model.add( y[t] >= (v - p[t]) );
        model.add( y[t] >= (p[t] - v) );
    }

    IloExpr obj(env);
    for (int t = 0; t < T; t++) {
        obj += y[t];
    }
    model.add(IloMinimize(env, obj));
    obj.end();

    // Solve
    IloCplex cplex(model);
    cplex.setOut(env.getNullStream());
    if (!cplex.solve()) {
        cerr << "[solveSingleAFO_BRP] No feasible solution.\n";
        env.end();
        return result;
    }

    // Extract solution
    result.finalSchedule.resize(T, 0.0);
    result.newDeviation.resize(T, 0.0);
    double sumOld = 0.0, sumNew = 0.0;

    for (int t = 0; t < T; t++) {
        double pVal = cplex.getValue(p[t]);
        double yVal = cplex.getValue(y[t]);
        result.finalSchedule[t] = pVal;
        result.newDeviation[t] = yVal;

        double old_m = fabs(mismatch[t + afo.get_aggregated_earliest_hour()]);
        sumOld += old_m;
        sumNew += yVal;
    }

    result.totalReduction = sumOld - sumNew;

    afo.apply_schedule(result.finalSchedule);

    env.end();

    return result;
}



SolverResult Solver::solveFCR_tec(AggregatedFlexOffer &afo, const vector<double> &mismatch)
{
    SolverResult result;
    result.totalReduction = 0.0;

    int T = afo.get_duration();


    auto profile = afo.get_aggregated_profile();
    double minOverall = afo.get_min_overall();
    double maxOverall = afo.get_max_overall();

    IloEnv env;
    IloModel model(env);
    IloNumVarArray p(env, T);
    for (int t = 0; t < T; t++) {
        double lb = profile[t].min_power;
        double ub = profile[t].max_power;
        p[t] = IloNumVar(env, lb, ub, ILOFLOAT);
    }

    IloNumVarArray y(env, T);
    for (int t = 0; t < T; t++) {
        y[t] = IloNumVar(env, 0.0, IloInfinity, ILOFLOAT);
    }

    for (int t = 0; t < T; t++) {
        double v = mismatch[t + afo.get_aggregated_earliest_hour()];

        model.add( y[t] >=  (v - p[t]) );
        model.add( y[t] >=  (p[t] - v) );
    }

    IloExpr sumP(env);
    for (int t = 0; t < T; t++) {
        sumP += p[t];
    }
    model.add(sumP >= minOverall);
    model.add(sumP <= maxOverall);
    sumP.end();

    IloExpr obj(env);
    for (int t = 0; t < T; t++) {
        obj += y[t];
    }
    model.add(IloMinimize(env, obj));
    obj.end();

    IloCplex cplex(model);
    cplex.setOut(env.getNullStream());
    if (!cplex.solve()) {
        cerr << "[solveFCR_tec] no feasible solution found.\n";
        env.end();
        return result;
    }

    result.finalSchedule.resize(T, 0.0);
    result.newDeviation.resize(T, 0.0);

    double sumOld = 0.0, sumNew = 0.0;
    for (int t = 0; t < T; t++) {
        double pVal = cplex.getValue(p[t]);
        double yVal = cplex.getValue(y[t]);


        result.finalSchedule[t] = pVal;
        result.newDeviation[t]  = yVal;

        double old_m = fabs(mismatch[t + afo.get_aggregated_earliest_hour()]);

        sumOld += old_m;
        sumNew += yVal;
    }
    result.totalReduction = sumOld - sumNew;

    afo.apply_schedule(result.finalSchedule);

    env.end();

    return result;
}