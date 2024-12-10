#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include "../include/flexoffer.h" 

using namespace std;
vector<double> compute_optimal_schedule(const Flexoffer &fo, const vector<double> &price_per_hour, double target_energy) {

    int duration = fo.get_duration();
    vector<TimeSlice> profile = fo.get_profile();

    vector<int> hours(duration);
    for (int i = 0; i < duration; i++)
        hours[i] = i;

    vector<double> schedule(duration, 0.0);

    double remaining_energy = target_energy;

    sort(hours.begin(), hours.end(), [&](int a, int b) {
        return price_per_hour[a] < price_per_hour[b]; 
    });


    // Assign energy starting from cheapest hour
    for (int i : hours) {
        double min_p = profile[i].min_power;
        double max_p = profile[i].max_power;

        // Maximum additional energy we can put in this hour (beyond min) is (max_p - min_p)
        double available_capacity = max_p - min_p;
        double allocate = 0.0;
        
        if (remaining_energy > 0.0) {
            if (remaining_energy >= available_capacity) {
                allocate = max_p; // take full capacity at this cheap hour
                remaining_energy -= available_capacity;
            } else {
                // Only need a partial allocation above min power to meet the target
                allocate = min_p + remaining_energy;
                remaining_energy = 0.0;
            }
        } else {
            allocate = min_p;
        }
        schedule[i] = allocate;
    }
    return schedule;
}


// Compute total cost given a schedule and price
double compute_total_cost(const Flexoffer &fo, const vector<double> &schedule, const vector<double> &price_per_hour) {
    double total_cost = 0.0;
    int duration = fo.get_duration();
    for (int i = 0; i < duration; i++) {
        total_cost += schedule[i] * price_per_hour[i];
    }
    return total_cost;
}

// Compute total cost
double compute_total_cost(const Flexoffer &fo, const vector<double> &price_per_hour) {
    vector<double> schedule = fo.get_scheduled_allocation();
    double total_cost = 0.0;
    int duration = fo.get_duration();
    for (int i = 0; i < duration; i++) {
        total_cost += schedule[i] * price_per_hour[i];
    }
    return total_cost;
}


// Compute a reasonable target energy (for demonstration)
double compute_target_energy(const Flexoffer &fo) {
    double total_min = 0.0;
    double total_max = 0.0;
    for (auto &ts : fo.get_profile()) {
        total_min += ts.min_power;
        total_max += ts.max_power;
    }
    return (total_min + total_max) / 2.0; // Halfway between total min and total max
}

// Compute flexibility utilization ratio (0 to 1)
double compute_flexibility_utilization(const Flexoffer &fo) {
    vector<TimeSlice> profile = fo.get_profile();
    vector<double> schedule = fo.get_scheduled_allocation();
    double total_available_flex = 0.0;
    double total_used_flex = 0.0;

    int duration = fo.get_duration();
    for (int i = 0; i < duration; i++) {
        double min_p = profile[i].min_power;
        double max_p = profile[i].max_power;
        double allocated = schedule[i];

        double available_range = max_p - min_p; 
        double used_range = allocated - min_p; // how far above min we actually went

        // Accumulate
        total_available_flex += available_range;
        total_used_flex += used_range;
    }

    // Handle the edge case where total_available_flex is zero (meaning no flexibility)
    if (total_available_flex <= 1e-9) {
        return 0.0; // no flexibility available
    }

    return total_used_flex / total_available_flex;
}