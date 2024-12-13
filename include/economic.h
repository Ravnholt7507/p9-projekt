#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include "flexoffer.h" 

using namespace std;
vector<double> compute_optimal_schedule(const Flexoffer &fo, const vector<double> &price_per_hour, double target_energy);

double compute_total_cost(const Flexoffer &fo, const vector<double> &schedule, const vector<double> &price_per_hour);
double compute_total_cost(const Flexoffer &fo, const vector<double> &price_per_hour);

double compute_target_energy(const Flexoffer &fo);

double compute_flexibility_utilization(const Flexoffer &fo);