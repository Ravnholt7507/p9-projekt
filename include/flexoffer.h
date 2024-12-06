#ifndef FLEXOFFER_H
#define FLEXOFFER_H

#include <ctime>
#include <vector>

struct TimeSlice {
    double min_power; // Minimum power in kW
    double max_power; // Maximum power in kW
};

class Flexoffer {
private:
    int offer_id;
    time_t earliest_start_time;
    time_t latest_start_time;
    time_t end_time;
    int duration;
    std::vector<TimeSlice> profile;
    std::vector<double> scheduled_allocation;
    time_t scheduled_start_time;

public:
    // Constructor
    Flexoffer(int oi, time_t est, time_t lst, time_t et, const std::vector<TimeSlice>& p, int d);

    // Getters
    int get_offer_id() const;
    time_t get_est() const;
    time_t get_lst() const;
    time_t get_et() const;
    int get_duration() const;
    const std::vector<TimeSlice>& get_profile() const;
    const std::vector<double>& get_scheduled_allocation() const;
    time_t get_scheduled_start_time() const;

    // Setters
    void set_scheduled_allocation(const std::vector<double>&);
    void set_scheduled_start_time(time_t);

    // Utils
    void print_flexoffer();

    // Additional methods
    int get_est_hour() const;
    int get_lst_hour() const;
    double get_total_energy() const;
};

#endif // FLEXOFFER_H
