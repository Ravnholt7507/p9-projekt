#include "../include/DFO.h"
#include <stdexcept>
#include <algorithm>

using namespace std;

// DependencyPolygon Constructor
DependencyPolygon::DependencyPolygon(double min_prev, double max_prev, int numsamples)
    : min_prev_energy(min_prev), max_prev_energy(max_prev), numsamples(numsamples) {}

// Add a point to the polygon
void DependencyPolygon::add_point(double x, double y) {
    points.push_back({x, y});
}

// Generate the convex polygon based on numsamples
void DependencyPolygon::generate_polygon() {
    double step = (max_prev_energy - min_prev_energy) / (numsamples - 1);

    for (int i = 0; i < numsamples; i++) {
        double current_prev_energy = min_prev_energy + i * step;

        double min_current_energy = 6.0 - 0.3 * current_prev_energy;
        double max_current_energy = 9.0 - 0.3 * current_prev_energy;

        // Ensure energy values are valid
        min_current_energy = max(min_current_energy, 0.0);
        max_current_energy = max(max_current_energy, 0.0);

        // Add the points to form the polygon
        add_point(current_prev_energy, min_current_energy);
        add_point(current_prev_energy, max_current_energy);
    }
}

// Print the polygon points
void DependencyPolygon::print_polygon() const {
    cout << "Dependency Polygon Points:" << endl;
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
    for (auto &polygon : polygons) {
        polygon.generate_polygon();
    }
}

// Print the entire DFO
void DFO::print_dfo() const {
    cout << "Dependency-Based Flexoffer ID: " << dfo_id << endl;
    for (const auto &polygon : polygons) {
        polygon.print_polygon();
    }
}

// Linear interpolation helper function
double linear_interpolation(double x, double x1, double y1, double x2, double y2) {
    return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}

// Function to aggregate two DFOs
DFO agg2to1(const DFO &dfo1, const DFO &dfo2, int numsamples) { //double &epsilon1, double &epsilon2
    if (dfo1.polygons.size() != dfo2.polygons.size()) {
        throw runtime_error("DFOs must have the same number of timesteps to aggregate. Kind Regards, agg2to1 function");
    }

    vector<DependencyPolygon> aggregated_polygons;
    //epsilon1 = 1.0;
    //epsilon2 = 1.0;

    for (size_t i = 0; i < dfo1.polygons.size(); ++i) {
        const auto &polygon1 = dfo1.polygons[i];
        const auto &polygon2 = dfo2.polygons[i];

        double aggregated_min_prev = polygon1.min_prev_energy + polygon2.min_prev_energy;
        double aggregated_max_prev = polygon1.max_prev_energy + polygon2.max_prev_energy;

        DependencyPolygon aggregated_polygon(aggregated_min_prev, aggregated_max_prev, numsamples);

        double step1 = (polygon1.max_prev_energy - polygon1.min_prev_energy) / (numsamples - 1);
        double step2 = (polygon2.max_prev_energy - polygon2.min_prev_energy) / (numsamples - 1);
        double step = (aggregated_max_prev - aggregated_min_prev) / (numsamples - 1);

        for (int j = 0; j < numsamples; ++j) {
            double current_prev_energy1 = polygon1.min_prev_energy + j * step1;
            double current_prev_energy2 = polygon2.min_prev_energy + j * step2;
            double current_prev_energy = aggregated_min_prev + j * step;

            double dfo1_min_energy = 0.0, dfo1_max_energy = 0.0;
            if (polygon1.numsamples == numsamples) {
                dfo1_min_energy = polygon1.points[j * 2].y;
                dfo1_max_energy = polygon1.points[j * 2 + 1].y;
            } else {
                for (size_t k = 1; k + 1 < polygon1.points.size(); k += 2) {
                    const auto &prev_point_min = polygon1.points[k - 1];
                    const auto &prev_point_max = polygon1.points[k];
                    const auto &next_point_min = polygon1.points[k + 1];
                    const auto &next_point_max = polygon1.points[k + 2];
                    if (current_prev_energy1 >= prev_point_min.x && current_prev_energy1 <= next_point_min.x) {
                        dfo1_min_energy = linear_interpolation(
                            current_prev_energy1, prev_point_min.x, prev_point_min.y, next_point_min.x, next_point_min.y);
                        dfo1_max_energy = linear_interpolation(
                            current_prev_energy1, prev_point_max.x, prev_point_max.y, next_point_max.x, next_point_max.y);
                        break;
                    }
                }
            }

            double dfo2_min_energy = 0.0, dfo2_max_energy = 0.0;
            if (polygon2.numsamples == numsamples) {
                dfo2_min_energy = polygon2.points[j * 2].y;
                dfo2_max_energy = polygon2.points[j * 2 + 1].y;
            } else {
                for (size_t k = 1; k + 1 < polygon2.points.size(); k += 2) {
                    const auto &prev_point_min = polygon2.points[k - 1];
                    const auto &prev_point_max = polygon2.points[k];
                    const auto &next_point_min = polygon2.points[k + 1];
                    const auto &next_point_max = polygon2.points[k + 2];
                    if (current_prev_energy2 >= prev_point_min.x && current_prev_energy2 <= next_point_min.x) {
                        dfo2_min_energy = linear_interpolation(
                            current_prev_energy2, prev_point_min.x, prev_point_min.y, next_point_min.x, next_point_min.y);
                        dfo2_max_energy = linear_interpolation(
                            current_prev_energy2, prev_point_max.x, prev_point_max.y, next_point_max.x, next_point_max.y);
                        break;
                    }
                }
            }

            double min_current_energy = dfo1_min_energy + dfo2_min_energy;
            double max_current_energy = dfo1_max_energy + dfo2_max_energy;

            aggregated_polygon.add_point(current_prev_energy, min_current_energy);
            aggregated_polygon.add_point(current_prev_energy, max_current_energy);

            /*double min_total_energy1 = current_prev_energy1 + dfo1_min_energy; // Some temporary code. 
            double max_total_energy1 = current_prev_energy1 + dfo1_max_energy;
            double min_total_energy2 = current_prev_energy2 + dfo2_min_energy;
            double max_total_energy2 = current_prev_energy2 + dfo2_max_energy;
            double used1_min;
            double used1_max;
            double used2_min;
            double used2_min;

            used1_max, used2_max = max(used1_max + used2_max) subject to ((used1_max - min_total_energy1) / (max_total_energy1 - min_total_energy1)) == ((used2_max - min_total_energy2) / (max_total_energy2 - min_total_energy2))
            used1_min, used2_min = min(used1_min + used2_min) subject to ((used1_min - min_total_energy1) / (max_total_energy1 - min_total_energy1)) == ((used2_min - min_total_energy2) / (max_total_energy2 - min_total_energy2))*/
        }

        aggregated_polygons.push_back(aggregated_polygon);
    }

    DFO aggregated_dfo(-1, {}, {}, numsamples);
    aggregated_dfo.polygons = aggregated_polygons;
    return aggregated_dfo;
}

// Function to aggregate multiple DFOs into one
DFO aggnto1(const vector<DFO> &dfos, int numsamples) {
    if (dfos.empty()) {
        throw runtime_error("No DFOs provided for aggregation. Kind Regards, aggnto1 function");
    }

    // Start aggregation with the first DFO
    DFO aggregated_dfo = dfos[0];

    // Aggregate subsequent DFOs
    for (size_t i = 1; i < dfos.size(); ++i) {
        aggregated_dfo = agg2to1(aggregated_dfo, dfos[i], numsamples);
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
        double s_min_A, s_max_A, s_min_1, s_max_1, s_min_2, s_max_2; // Total energy bounds for the DFOs at this timestep

        // Find points with the respective energy dependency for DFO A, 1 and 2 to calculate allowed min and max energy usage for each
        vector<Point> matching_pointsA, matching_points1, matching_points2;
        // For the aggregated DFO
        for (const auto &point : polygonA.points) { // Try to find if there points for that exact amount of dependece energy
            if (point.x == dA) { // Check if the x-coordinate matches
                matching_pointsA.push_back(point);
            }
        }

        if(matching_pointsA.empty()) { // If there is no points with that exact amount of dependence energy, use linear interpolation between point before and after
            for (size_t k = 1; k + 1 < polygonA.points.size(); k += 2) {
                    const auto &prev_point_min = polygonA.points[k - 1];
                    const auto &prev_point_max = polygonA.points[k];
                    const auto &next_point_min = polygonA.points[k + 1];
                    const auto &next_point_max = polygonA.points[k + 2];
                    if (dA >= prev_point_min.x && dA <= next_point_min.x) {
                        s_min_A = linear_interpolation(
                            dA, prev_point_min.x, prev_point_min.y, next_point_min.x, next_point_min.y);
                        s_max_A = linear_interpolation(
                            dA, prev_point_max.x, prev_point_max.y, next_point_max.x, next_point_max.y);
                        break;
                    }
            }
            matching_pointsA.push_back({dA, s_min_A});
            matching_pointsA.push_back({dA, s_max_A});
        }

        // For DFO 1
        for (const auto &point : polygon1.points) { // Try to find if there points for that exact amount of dependece energy
            if (point.x == d1) { // Check if the x-coordinate matches
                matching_points1.push_back(point);
            }
        }

        if(matching_points1.empty()) { // If there is no points with that exact amount of dependence energy, use linear interpolation between point before and after
            for (size_t k = 1; k + 1 < polygon1.points.size(); k += 2) {
                    const auto &prev_point_min = polygon1.points[k - 1];
                    const auto &prev_point_max = polygon1.points[k];
                    const auto &next_point_min = polygon1.points[k + 1];
                    const auto &next_point_max = polygon1.points[k + 2];
                    if (d1 >= prev_point_min.x && d1 <= next_point_min.x) {
                        s_min_1 = linear_interpolation(
                            d1, prev_point_min.x, prev_point_min.y, next_point_min.x, next_point_min.y);
                        s_max_1 = linear_interpolation(
                            d1, prev_point_max.x, prev_point_max.y, next_point_max.x, next_point_max.y);
                        break;
                    }
            }
            matching_points1.push_back({d1, s_min_1});
            matching_points1.push_back({d1, s_max_1});
        }

        // For DFO 2
        for (const auto &point : polygon2.points) { // Try to find if there points for that exact amount of dependece energy
            if (point.x == d2) { // Check if the x-coordinate matches
                matching_points2.push_back(point);
            }
        }

        if(matching_points2.empty()) { // If there is no points with that exact amount of dependence energy, use linear interpolation between point before and after
            for (size_t k = 1; k + 1 < polygon2.points.size(); k += 2) {
                    const auto &prev_point_min = polygon2.points[k - 1];
                    const auto &prev_point_max = polygon2.points[k];
                    const auto &next_point_min = polygon2.points[k + 1];
                    const auto &next_point_max = polygon2.points[k + 2];
                    if (d2 >= prev_point_min.x && d2 <= next_point_min.x) {
                        s_min_2 = linear_interpolation(
                            d2, prev_point_min.x, prev_point_min.y, next_point_min.x, next_point_min.y);
                        s_max_2 = linear_interpolation(
                            d2, prev_point_max.x, prev_point_max.y, next_point_max.x, next_point_max.y);
                        break;
                    }
            }
            matching_points2.push_back({d2, s_min_2});
            matching_points2.push_back({d2, s_max_2});
        }

        // Calculate scaling factor between min and max energy used for this timestep in aggregated DFO based on reference schedule
        const Point &point1 = matching_pointsA[0];
        const Point &point2 = matching_pointsA[1];
        if (s_max_A - s_min_A == 0) {
            f = 0;
        } else {
            f = (yA_ref[i] - point1.y) / (point2.y - point1.y);
        }

        // Use same scaling factor on DFO 1 and 2 respectively, to get their corresponding energy usage
        // DFO1
        const Point &point1 = matching_points1[0];
        const Point &point2 = matching_points1[1];
        y1_ref[i] = point1.y + f * (point2.y - point1.y);

        // DFO2 
        const Point &point1 = matching_points2[0];
        const Point &point2 = matching_points2[1];
        y2_ref[i] = point1.y + f * (point2.y - point1.y);

        //Update dependency amounts
        dA += yA_ref[i];
        d1 += y1_ref[i];
        d2 += y2_ref[i];
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
*/