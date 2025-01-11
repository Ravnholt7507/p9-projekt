#include "../include/DFO.h"
#include <stdexcept>
#include <algorithm>
#include <ilcplex/ilocplex.h>

#include <ostream> // Required for std::ostream

// Overload the << operator for the Point struct
std::ostream &operator<<(std::ostream &os, const Point &point) {
    os << "(" << point.x << ", " << point.y << ")";
    return os;
}

using namespace std;

// DependencyPolygon Constructor
DependencyPolygon::DependencyPolygon(double min_prev, double max_prev, int numsamples)
    : min_prev_energy(min_prev), max_prev_energy(max_prev), numsamples(numsamples) {}

// Add a point to the polygon
void DependencyPolygon::add_point(double x, double y) {
    points.push_back({x, y});
}

// Generate the convex polygon based on numsamples
void DependencyPolygon::generate_polygon(size_t i, double next_min_prev, double next_max_prev) {
    if (min_prev_energy == max_prev_energy) {
        double min_current_energy = max(next_min_prev - min_prev_energy, 0.0);
        double max_current_energy = max(next_max_prev - min_prev_energy, 0.0);
        
        add_point(min_prev_energy, min_current_energy);
        add_point(max_prev_energy, max_current_energy);
        return;
    }
    
    double step = (max_prev_energy - min_prev_energy) / (numsamples - 1);

    for (int i = 0; i < numsamples; i++) {
        double current_prev_energy = min_prev_energy + i * step;

        // Calculate the min and max energy needed for the next time slice
        double min_current_energy = max(next_min_prev - current_prev_energy, 0.0);
        double max_current_energy = max(next_max_prev - current_prev_energy, 0.0);

        // Add the points to the polygon
        add_point(current_prev_energy, min_current_energy);
        add_point(current_prev_energy, max_current_energy);
    }

    sort_points();
}

// Overload for the last timestep (no next slice)
void DependencyPolygon::generate_last_polygon() {
    double step = (max_prev_energy - min_prev_energy) / (numsamples - 1);

    for (int i = 0; i < numsamples; i++) {
        double current_prev_energy = min_prev_energy + i * step;

        // Add arbitrary values for the last time slice
        double min_current_energy = 0.0;
        double max_current_energy = 10.0; // Example value

        add_point(current_prev_energy, min_current_energy);
        add_point(current_prev_energy, max_current_energy);
    }
}

void DependencyPolygon::sort_points() {
    std::sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
        if (a.x == b.x) {
            return a.y < b.y; // Sort by y when x values are equal
        }
        return a.x < b.x; // Sort by x by default
    });
}

// Print the polygon points
void DependencyPolygon::print_polygon(int i) const {
    cout << "Dependency Polygon Points at hour " << i << ":" << endl;
    for (const auto &point : points) {
        cout << "(" << point.x << ", " << point.y << ")" << endl;
    }
}

// DFO Constructor
DFO::DFO(int id, vector<double> min_prev, vector<double> max_prev, int numsamples)
    : dfo_id(id) {
    // Create polygons for each timestep
    for (size_t i = 0; i < min_prev.size(); ++i) {
        DependencyPolygon polygon(min_prev[i], max_prev[i], numsamples);
        polygons.push_back(polygon);
    }
}

// Generate all dependency polygons
void DFO::generate_dependency_polygons() {
    for (size_t i = 0; i < polygons.size(); ++i) {
        if (i < polygons.size() - 1) {
            polygons[i].generate_polygon(i, polygons[i + 1].min_prev_energy, polygons[i + 1].max_prev_energy);
        } else {
            polygons[i].generate_polygon(i, 0, 0); // Last timestep
        }
    }
}

// Print the entire DFO
void DFO::print_dfo() const {
    cout << "Dependency-Based Flexoffer ID: " << dfo_id << endl;
    for (size_t i = 0; i < polygons.size(); ++i) {
        polygons[i].print_polygon(i);
    }
}

// Linear interpolation helper function
double linear_interpolation(double x, double x1, double y1, double x2, double y2) {
    return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}

vector<Point> find_or_interpolate_points(
    const vector<Point>& points, 
    double dependency_value
) {
    vector<Point> matching_points;
    double s_min = 0.0, s_max = 0.0;

    // Try to find if there are points for that exact amount of dependence energy
    for (const auto &point : points) {
        if (point.x == dependency_value) {
            matching_points.push_back(point);
        }
    }

    // If there is no points with that exact amount of dependence energy, use linear interpolation between points before and after
    if (matching_points.empty()) {
        for (size_t k = 1; k + 1 < points.size(); k += 2) {
            const auto &prev_point_min = points[k - 1];
            const auto &prev_point_max = points[k];
            const auto &next_point_min = points[k + 1];
            const auto &next_point_max = points[k + 2];
            if (dependency_value >= prev_point_min.x && dependency_value <= next_point_min.x) {
                s_min = linear_interpolation(
                    dependency_value, prev_point_min.x, prev_point_min.y, next_point_min.x, next_point_min.y);
                s_max = linear_interpolation(
                    dependency_value, prev_point_max.x, prev_point_max.y, next_point_max.x, next_point_max.y);
                break;
            }
        }
        matching_points.push_back({dependency_value, s_min});
        matching_points.push_back({dependency_value, s_max});
    }

    return matching_points;
}

// Function to aggregate two DFOs
DFO agg2to1(const DFO &dfo1, const DFO &dfo2, int numsamples, double &epsilon1, double &epsilon2) {
    if (dfo1.polygons.size() != dfo2.polygons.size()) {
        throw runtime_error("DFOs must have the same number of timesteps to aggregate. Kind Regards, agg2to1 function");
    }

    vector<DependencyPolygon> aggregated_polygons;
    epsilon1 = 1.0;
    epsilon2 = 1.0;

    for (size_t i = 0; i < dfo1.polygons.size(); ++i) {
        const auto &polygon1 = dfo1.polygons[i];
        const auto &polygon2 = dfo2.polygons[i];

        double aggregated_min_prev = polygon1.min_prev_energy + polygon2.min_prev_energy;
        double aggregated_max_prev = polygon1.max_prev_energy + polygon2.max_prev_energy;

        DependencyPolygon aggregated_polygon(aggregated_min_prev, aggregated_max_prev, numsamples);
        
        double min_current_energy, max_current_energy;
        if (polygon1.points.size() == 2 && polygon2.points.size() == 2) { // Special case - For example if 1st timestep and the dependency min/max is 0. Then there are only 2 points
            min_current_energy = polygon1.points[0].y + polygon2.points[0].y;
            max_current_energy = polygon1.points[1].y + polygon2.points[1].y;
            double dependency_Amount = polygon1.points[1].x + polygon2.points[1].x;

            aggregated_polygon.add_point(dependency_Amount, min_current_energy);
            aggregated_polygon.add_point(dependency_Amount, max_current_energy);
        } else { // Not special case. Loop through from dependency min to dependency max with step size based on numSamples
            double step1 = (polygon1.max_prev_energy - polygon1.min_prev_energy) / (numsamples - 1);
            double step2 = (polygon2.max_prev_energy - polygon2.min_prev_energy) / (numsamples - 1);
            double step = (aggregated_max_prev - aggregated_min_prev) / (numsamples - 1);
            
            for (int j = 0; j < numsamples; ++j) {
                // Calculate current dependency amount for DFO1, DFO2, and aggregated DFO
                double current_prev_energy1 = polygon1.min_prev_energy + j * step1;
                double current_prev_energy2 = polygon2.min_prev_energy + j * step2;
                double current_prev_energy = aggregated_min_prev + j * step;

                // Based on the current dependency amount - find min and max energy usage for DFO1 and DFO2 respectively
                // DFO 1
                vector<Point> matching_points1 = find_or_interpolate_points(polygon1.points, current_prev_energy1);
                double dfo1_min_energy = matching_points1[0].y;
                double dfo1_max_energy = matching_points1[1].y;
                
                // DFO 2
                vector<Point> matching_points2 = find_or_interpolate_points(polygon2.points, current_prev_energy2);
                double dfo2_min_energy = matching_points2[0].y;
                double dfo2_max_energy = matching_points2[1].y;

                // Aggregate the found min / max energy amount and add the points to the aggregated DFO slice
                if(i+1 == dfo1.polygons.size()) {
                    min_current_energy = dfo1_min_energy + dfo2_min_energy;
                    max_current_energy = dfo1_max_energy + dfo2_max_energy;
                } else {
                    double min_total_energy1 = current_prev_energy1 + dfo1_min_energy; 
                    double max_total_energy1 = current_prev_energy1 + dfo1_max_energy;
                    double min_total_energy2 = current_prev_energy2 + dfo2_min_energy;
                    double max_total_energy2 = current_prev_energy2 + dfo2_max_energy;
                    double used1_min_final = 0.0, used2_min_final = 0.0;
                    double used1_max_final = 0.0, used2_max_final = 0.0;

                    IloEnv env;
                    try {
                        IloModel model(env);
                        // Decision variables
                        IloNumVar used1_min(env, 0.0, IloInfinity, ILOFLOAT);
                        IloNumVar used1_max(env, 0.0, IloInfinity, ILOFLOAT);
                        IloNumVar used2_min(env, 0.0, IloInfinity, ILOFLOAT);
                        IloNumVar used2_max(env, 0.0, IloInfinity, ILOFLOAT);
                        // Add constraints for maximums
                        model.add(
                            (used1_max - min_total_energy1) * (max_total_energy2 - min_total_energy2) ==
                            (used2_max - min_total_energy2) * (max_total_energy1 - min_total_energy1)
                        );
                        // Add constraints for minimums
                        model.add(
                            (used1_min - min_total_energy1) * (max_total_energy2 - min_total_energy2) ==
                            (used2_min - min_total_energy2) * (max_total_energy1 - min_total_energy1)
                        );
                        // Add sum constraints
                        model.add(used1_max + used2_max <= max_total_energy1 + max_total_energy2);
                        model.add(used1_min + used2_min >= min_total_energy1 + min_total_energy2);
                        // **Minimize `used1_min` and `used2_min`**
                        IloCplex cplex_min(model);
                        cplex_min.setOut(env.getNullStream());
                        IloObjective minimizeObjective(env, used1_min + used2_min, IloObjective::Minimize);
                        model.add(minimizeObjective);
                        if (cplex_min.solve()) {
                            used1_min_final = cplex_min.getValue(used1_min);
                            used2_min_final = cplex_min.getValue(used2_min);
                        } else {
                            std::cerr << "No solution found during minimization!" << std::endl;
                        }
                        model.remove(minimizeObjective);
                        
                        // **Maximize `used1_max` and `used2_max`**
                        IloCplex cplex_max(model);
                        cplex_max.setOut(env.getNullStream());
                        IloObjective maximizeObjective(env, used1_max + used2_max, IloObjective::Maximize);
                        model.add(maximizeObjective);
                        
                        if (cplex_max.solve()) {
                            used1_max_final = cplex_max.getValue(used1_max);
                            used2_max_final = cplex_max.getValue(used2_max);
                        } else {
                            std::cerr << "No solution found during maximization!" << std::endl;
                        }
                    } catch (const IloException &e) {
                        std::cerr << "Error: " << e.getMessage() << std::endl;
                    } catch (...) {
                        std::cerr << "Unknown error occurred!" << std::endl;
                    }
                    env.end();

                    min_current_energy = used1_min_final + used2_min_final;
                    max_current_energy = used1_max_final + used2_max_final;
                    if (dfo1_max_energy != dfo1_min_energy) {
                        epsilon1 *= (used1_max_final - used1_min_final) / (dfo1_max_energy - dfo1_min_energy);
                    }
                    if (dfo2_max_energy != dfo2_min_energy) {
                        epsilon2 *= (used2_max_final - used2_min_final) / (dfo2_max_energy - dfo2_min_energy);
                    }

                }
                aggregated_polygon.add_point(current_prev_energy, min_current_energy - current_prev_energy);
                aggregated_polygon.add_point(current_prev_energy, max_current_energy - current_prev_energy);
            }
        }
        aggregated_polygon.sort_points();

        aggregated_polygons.push_back(aggregated_polygon);
    }

    DFO aggregated_dfo(-1, {}, {}, numsamples);
    aggregated_dfo.polygons = aggregated_polygons;
    return aggregated_dfo;
}

// Function to aggregate multiple DFOs into one
DFO aggnto1(const vector<DFO> &dfos, int numsamples, double &epsilon1, double &epsilon2) {
    if (dfos.empty()) {
        throw runtime_error("No DFOs provided for aggregation. Kind Regards, aggnto1 function");
    }

    // Start aggregation with the first DFO
    DFO aggregated_dfo = dfos[0];

    // Aggregate subsequent DFOs
    for (size_t i = 1; i < dfos.size(); ++i) {
        aggregated_dfo = agg2to1(aggregated_dfo, dfos[i], numsamples, epsilon1, epsilon2);
    }

    return aggregated_dfo;
}

void disagg1to2(
    const DFO &D1, const DFO &D2, const DFO &DA, 
    const std::vector<double> &yA_ref, 
    std::vector<double> &y1_ref, std::vector<double> &y2_ref) {

    size_t T = DA.polygons.size(); // Number of timesteps
    if (T != yA_ref.size()) {
        throw std::runtime_error("Mismatch between DA timesteps and yA_ref size. Kind regards disagg1to2 function");
    }

    // Initialize energy dependency amounts
    double dA = 0.0, d1 = 0.0, d2 = 0.0;

    // Resize the output vectors to match the number of timesteps
    y1_ref.resize(T, 0.0);
    y2_ref.resize(T, 0.0);

    for (size_t i = 0; i < DA.polygons.size(); ++i) {
        // Get DFO slice for that timestep
        const auto &polygonA = DA.polygons[i];
        const auto &polygon1 = D1.polygons[i];
        const auto &polygon2 = D2.polygons[i];

        double f = 0.0; // Initialize splitting factor

        // Find points with the respective energy dependency for DFO A, 1 and 2 to calculate allowed min and max energy usage for each
        vector<Point> matching_pointsA, matching_points1, matching_points2;
        // For the aggregated DFO
        matching_pointsA = find_or_interpolate_points(polygonA.points, dA);

        // For DFO 1
        matching_points1 = find_or_interpolate_points(polygon1.points, d1);

        // For DFO 2
        matching_points2 = find_or_interpolate_points(polygon2.points, d2);

        // Calculate scaling factor between min and max energy used for this timestep in aggregated DFO based on reference schedule
        const Point &pointA_1 = matching_pointsA[0];
        const Point &pointA_2 = matching_pointsA[1];
        if (pointA_2.y - pointA_1.y == 0) {
            f = 0;
        } else {
            f = (yA_ref[i] - pointA_1.y) / (pointA_2.y - pointA_1.y);
        }

        // Use same scaling factor on DFO 1 and 2 respectively, to get their corresponding energy usage
        // DFO1
        const Point &point1_1 = matching_points1[0];
        const Point &point1_2 = matching_points1[1];
        y1_ref[i] = point1_1.y + f * (point1_2.y - point1_1.y);

        // DFO2 
        const Point &point2_1 = matching_points2[0];
        const Point &point2_2 = matching_points2[1];
        y2_ref[i] = point2_1.y + f * (point2_2.y - point2_1.y);

        //Update dependency amounts
        dA += yA_ref[i];
        d1 += y1_ref[i];
        d2 += y2_ref[i];
    }
}

void disagg1toN(
    const DFO &DA, const std::vector<DFO> &DFOs, 
    const std::vector<double> &yA_ref, 
    std::vector<std::vector<double>> &y_refs) {

    size_t T = DA.polygons.size(); // Number of timesteps
    size_t N = DFOs.size();        // Number of DFOs to disaggregate into

    if (T != yA_ref.size()) {
        throw std::runtime_error("Mismatch between DA timesteps and yA_ref size. Kind regards disagg1toN function");
    }

    // Initialize energy dependency amounts for all DFOs
    std::vector<double> d(N, 0.0);  // Dependency amounts for DFOs
    double dA = 0.0;                // Dependency amount for aggregated DFO

    // Resize the output vectors to match the number of timesteps
    y_refs.resize(N, std::vector<double>(T, 0.0));

    for (size_t i = 0; i < T; ++i) {
        // Get DFO slice for the timestep
        const auto &polygonA = DA.polygons[i];

        double f = 0.0; // Initialize splitting factor

        // Find points with the respective energy dependency for DFO A to calculate allowed min and max energy usage
        std::vector<Point> matching_pointsA = find_or_interpolate_points(polygonA.points, dA);

        // Calculate scaling factor between min and max energy used for this timestep in aggregated DFO based on reference schedule
        const Point &pointA1 = matching_pointsA[0];
        const Point &pointA2 = matching_pointsA[1];
        if (pointA2.y - pointA1.y == 0) {
            f = 0;
        } else {
            f = (yA_ref[i] - pointA1.y) / (pointA2.y - pointA1.y);
        }

        // Use the scaling factor on all DFOs
        for (size_t j = 0; j < N; ++j) { // Loop through all original non-aggregated DFOs
            const auto &polygon = DFOs[j].polygons[i];

            std::vector<Point> matching_points = find_or_interpolate_points(polygon.points, d[j]);

            const Point &point1 = matching_points[0];
            const Point &point2 = matching_points[1];
            y_refs[j][i] = point1.y + f * (point2.y - point1.y);

            // Update dependency amount for the current DFO
            d[j] += y_refs[j][i];
        }

        // Update dependency amount for the aggregated DFO
        dA += yA_ref[i];
    }
}

/*
Example main usage:

#include "../include/DFO.h"

int main(){
    vector<DFO> dfos;
    int numsamples = 5;

    // Example DFOs with identical timesteps for simplicity
    dfos.push_back(DFO(1, {10.0, 15.0}, {20.0, 25.0}, numsamples));
    dfos.push_back(DFO(2, {5.0, 10.0}, {15.0, 20.0}, numsamples));
    dfos.push_back(DFO(3, {8.0, 12.0}, {18.0, 22.0}, numsamples));

    // Generate dependency polygons for each DFO
    for (auto &dfo : dfos) {
        dfo.generate_dependency_polygons();
        dfo.print_dfo();
    }

    // Aggregate all DFOs
    try {
        DFO aggregated_dfo = aggnto1(dfos, numsamples);
        aggregated_dfo.print_dfo();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

or

int main() {
    // Initialize DFO 1
    std::vector<double> min_prev_1 = {0.0, 2.0, 4.0};
    std::vector<double> max_prev_1 = {2.0, 4.0, 6.0};
    int numsamples = 3; // Number of samples per polygon
    // Manually define DFO 1
    DFO dfo1(1, {}, {}, numsamples);
    dfo1.polygons = {
        DependencyPolygon(0.0, 0.0, numsamples),  // Timestep 1
        DependencyPolygon(2.0, 4.0, numsamples),  // Timestep 2
        DependencyPolygon(4.0, 6.0, numsamples)   // Timestep 3
    };

    dfo1.polygons[0].points = {{0.0, 2.0}, {0.0, 4.0}};
    dfo1.polygons[1].points = {{2.0, 2.0}, {2.0, 4.0}, {3.0, 1.0},  {3.0, 3.0}, {4.0, 0.0}, {4.0, 2.0}};
    dfo1.polygons[2].points = {{4.0, 3.0}, {4.0, 6.0}, {5.0, 2.0}, {5.0, 3.0}, {6.0, 1.0}, {6.0, 4.0}};

    // Manually define DFO 2
    DFO dfo2(2, {}, {}, numsamples);
    dfo2.polygons = {
        DependencyPolygon(0.0, 0.0, numsamples),  // Timestep 1
        DependencyPolygon(3.0, 5.0, numsamples),  // Timestep 2
        DependencyPolygon(5.0, 7.0, numsamples)   // Timestep 3
    };

    dfo2.polygons[0].points = {{0.0, 3.0}, {0.0, 5.0}};
    dfo2.polygons[1].points = {{3.0, 2.0}, {3.0, 4.0}, {4.0, 1.0}, {4.0, 3.0}, {5.0, 0.0}, {5.0, 2.0}};
    dfo2.polygons[2].points = {{5.0, 4.0}, {5.0, 6.0}, {6.0, 3.0}, {6.0, 5.0}, {7.0, 2.0}, {7.0, 4.0}};
    // Aggregate DFO 1 and DFO 2
    DFO aggregated_dfo = agg2to1(dfo1, dfo2, numsamples);
    std::cout << "Aggregated DFO:" << std::endl;
    aggregated_dfo.print_dfo();

    // Reference schedule for disaggregation
    std::vector<double> yA_ref = {6.0, 6.0, 8.0}; // Example energy schedule for aggregated DFO

    // Disaggregate aggregated DFO into DFO 1 and DFO 2
    std::vector<double> y1_ref, y2_ref;
    disagg1to2(dfo1, dfo2, aggregated_dfo, yA_ref, y1_ref, y2_ref);

    std::cout << "\nDisaggregated energy schedule for DFO 1:" << std::endl;
    for (const auto &val : y1_ref) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << "Disaggregated energy schedule for DFO 2:" << std::endl;
    for (const auto &val : y2_ref) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // Disaggregate aggregated DFO into multiple DFOs
    std::vector<DFO> dfos = {dfo1, dfo2};
    std::vector<std::vector<double>> y_refs;
    disagg1toN(aggregated_dfo, dfos, yA_ref, y_refs);

    std::cout << "\nDisaggregated energy schedules for multiple DFOs:" << std::endl;
    for (size_t i = 0; i < y_refs.size(); ++i) {
        std::cout << "DFO " << i + 1 << ": ";
        for (const auto &val : y_refs[i]) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}


SOLVER in main::

int main() {
    // Initialize DFO 1
    vector<double> min_prev_1 = {0.0, 2.0, 4.0};
    vector<double> max_prev_1 = {0.0, 4.0, 6.0};
    int numsamples = 5;

    DFO dfo1(1, min_prev_1, max_prev_1, numsamples);
    dfo1.generate_dependency_polygons();

    //dfo1.print_dfo();

    vector<double> min_prev_2 = {0.0, 5.0, 8.0};
    vector<double> max_prev_2 = {0.0, 8.0, 12.0};

    DFO dfo2(1, min_prev_2, max_prev_2, numsamples);
    dfo2.generate_dependency_polygons();

    //dfo2.print_dfo();
    // Aggregate DFO 1 and DFO 2
    DFO aggregated_dfo = agg2to1(dfo1, dfo2, numsamples);
    std::cout << "Aggregated DFO:" << std::endl;
    aggregated_dfo.print_dfo();



vector<double> spot_prices = {4.0, 5.0, 3.0};
    Solver::DFO_Optimization(aggregated_dfo, spot_prices);
/*
    // Reference schedule for disaggregation
    std::vector<double> yA_ref = {7.5, 7.0, 5.0}; // Example energy schedule for aggregated DFO

    // Disaggregate aggregated DFO into DFO 1 and DFO 2
    std::vector<double> y1_ref, y2_ref;
    disagg1to2(dfo1, dfo2, aggregated_dfo, yA_ref, y1_ref, y2_ref);

    std::cout << "\nDisaggregated energy schedule for DFO 1:" << std::endl;
    for (const auto &val : y1_ref) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << "Disaggregated energy schedule for DFO 2:" << std::endl;
    for (const auto &val : y2_ref) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // Disaggregate aggregated DFO into multiple DFOs
    std::vector<DFO> dfos = {dfo1, dfo2};
    std::vector<std::vector<double>> y_refs;
    disagg1toN(aggregated_dfo, dfos, yA_ref, y_refs);

    std::cout << "\nDisaggregated energy schedules for multiple DFOs:" << std::endl;
    for (size_t i = 0; i < y_refs.size(); ++i) {
        std::cout << "DFO " << i + 1 << ": ";
        for (const auto &val : y_refs[i]) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}

*/
