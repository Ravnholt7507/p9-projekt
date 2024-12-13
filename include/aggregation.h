#ifndef AGGREGATION_H
#define AGGREGATION_H

#include <ctime>
#include <limits>
#include <vector>
#include <chrono>
#include <algorithm>
#include <stdexcept>
#include "flexoffer.h"

// struct TimeSlice {
//     double min_power;
//     double max_power;
// };

class AggregatedFlexOffer {
private:
    int id;
    time_t aggregated_earliest;
    time_t aggregated_latest;
    time_t aggregated_end_time;
    std::vector<TimeSlice> aggregated_profile;
    std::vector<double> scheduled_allocation;
    int duration; 
    std::vector<Flexoffer> individual_offers;

public:
    // Constructor
    AggregatedFlexOffer(int offer_id, const std::vector<Flexoffer> &offers);

    // Getters
    int get_id() const;
    time_t get_aggregated_earliest() const;
    time_t get_aggregated_latest() const;
    time_t get_aggregated_end_time() const;
    std::vector<TimeSlice> get_aggregated_profile() const;
    const std::vector<double>& get_scheduled_allocation() const;
    int get_duration() const; 
    std::vector<Flexoffer> get_individual_offers() const;

    int get_aggregated_earliest_hour() const;
    int get_aggregated_latest_hour() const;
    int get_aggregated_end_time_hour() const;

    // Setters
    void set_id(int);
    void set_aggregated_earliest(time_t);
    void set_aggregated_latest(time_t);
    void set_aggregated_end_time(time_t);
    void set_aggregated_profile(const std::vector<TimeSlice>&);
    void set_scheduled_allocation(const std::vector<double>&);
    void set_duration(int); 
    void set_individual_offers(const std::vector<Flexoffer>&);

    // Utils
    void pretty_print() const;

    // Method to apply a given schedule allocation
    void apply_schedule(const std::vector<double> &allocations);

    // Method to pad the profile to a given duration
    void pad_profile(int target_duration);
};

#endif
