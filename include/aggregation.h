#ifndef AGGREGATION_H
#define AGGREGATION_H
#include <ctime>
#include <vector>
#include <variant>

#include "flexoffer.h"
#include "tec.h"
#include "alignments.h"

using namespace std;

class AggregatedFlexOffer {
private:
    int id;
    time_t aggregated_earliest;
    time_t aggregated_latest;
    time_t aggregated_end_time;
    vector<TimeSlice> aggregated_profile;
    vector<double> scheduled_allocation;
    vector<variant<Flexoffer, Tec_flexoffer>> individual_offers; 
    int duration; 
    double overall_min{0};
    double overall_max{0};

public:
    // Constructor
    AggregatedFlexOffer(const int offer_id, const Alignments a, const vector<Flexoffer> &offers);
    AggregatedFlexOffer(const int offer_id, const Alignments a, const vector<Tec_flexoffer> &offers);

    // Getters
    int get_id() const;
    time_t get_aggregated_earliest() const;
    time_t get_aggregated_latest() const;
    time_t get_aggregated_end_time() const;
    vector<TimeSlice> get_aggregated_profile() const;
    const vector<double>& get_scheduled_allocation() const;
    int get_duration() const; 
    vector<variant<Flexoffer, Tec_flexoffer>> get_individual_offers() const;
    double get_min_overall() const;
    double get_max_overall() const;

    // Hour-based getters
    int get_aggregated_earliest_hour() const;
    int get_aggregated_latest_hour() const;
    int get_aggregated_end_time_hour() const;

    // Setters
    void set_id(int);
    void set_aggregated_earliest(time_t);
    void set_aggregated_latest(time_t);
    void set_aggregated_end_time(time_t);
    void set_aggregated_profile(const vector<TimeSlice>&);
    void set_scheduled_allocation(const vector<double>&);
    void set_duration(int); 
    void set_individual_offers(const vector<variant<Flexoffer, Tec_flexoffer>>&);

    // Utils
    void pretty_print() const;

    // Method to apply a given schedule allocation
    void apply_schedule(const vector<double> &allocations);

    //disaggregation
    vector<Flexoffer> disaggregate_to_flexoffers();
};


#endif
