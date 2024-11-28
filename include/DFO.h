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
        void generate_polygon();
        void add_point(double x, double y);
        void print_polygon() const;

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

DFO agg2to1(const DFO &dfo1, const DFO &dfo2, int numsamples);
DFO aggnto1(const vector<DFO> &dfos, int numsamples);

#endif