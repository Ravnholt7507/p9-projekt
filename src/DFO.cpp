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
DFO agg2to1(const DFO &dfo1, const DFO &dfo2, int numsamples) {
    if (dfo1.polygons.size() != dfo2.polygons.size()) {
        throw runtime_error("DFOs must have the same number of timesteps to aggregate. Kind Regards, agg2to1 function");
    }

    vector<DependencyPolygon> aggregated_polygons;

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