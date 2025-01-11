#ifndef DFO_H
#define DFO_H
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

struct Point {
    double x; // Total energy from previous timesteps
    double y; // Energy usage in the current timestep; either min or max
};

class DependencyPolygon {
    public:
        DependencyPolygon(double min_prev, double max_prev, int numsamples);
        void generate_polygon(size_t i, double next_min_prev, double next_max_prev);
        void generate_last_polygon();
        void add_point(double x, double y);
        void print_polygon(int i) const;

        vector<Point> points;
        double min_prev_energy;
        double max_prev_energy;
        int numsamples;
};

class DFO {
    public:
        DFO(int id, vector<double> min_prev, vector<double> max_prev, int numsamples);
        void generate_dependency_polygons();
        void print_dfo() const;

        int dfo_id;
        vector<DependencyPolygon> polygons;
};

DFO agg2to1(const DFO &dfo1, const DFO &dfo2, int numsamples, double &epsilon1, double &epsilon2);
DFO aggnto1(const vector<DFO> &dfos, int numsamples);
void disagg1to2(
    const DFO &D1, const DFO &D2, const DFO &DA, 
    const std::vector<double> &yA_ref, 
    std::vector<double> &y1_ref, std::vector<double> &y2_ref);
void disagg1toN(
    const DFO &DA, const std::vector<DFO> &DFOs, 
    const std::vector<double> &yA_ref, 
    std::vector<std::vector<double>> &y_refs);
vector<Point> find_or_interpolate_points(
    const vector<Point>& points, 
    double dependency_value
);

#endif
