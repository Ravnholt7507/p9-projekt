#ifndef AGGREGATION_H
#define AGGREGATION_H
#include <ctime>
#include <limits>
#include <vector>
#include <chrono>

#include "flexoffer.h"


class AggregatedFlexOffer{
    private: 
        int id;
        time_t aggregated_earliest = std::numeric_limits<int>::max();
        time_t aggregated_latest= std::numeric_limits<int>::max();
        time_t aggregated_end_time = 0;
        vector<TimeSlice> aggregated_profile;
        double scheduled_allocation[24] = {0};
        int duration; 
        vector<Flexoffer> individual_offers;
    public:
        //Constructor
        AggregatedFlexOffer(int offer_id, vector<Flexoffer> &offers);

        //Getters
        int get_id() const;
        time_t get_aggregated_earliest() const;
        time_t get_aggregated_latest() const;
        time_t get_aggregated_end_time() const;
        vector<TimeSlice> get_aggregated_profile() const;
        double* get_scheduled_allocation();
        int get_duration() const; 
        vector<Flexoffer> get_individual_offers() const;

        //Setters
        void set_id(int);
        void set_aggregated_earliest(time_t);
        void set_aggregated_latest(time_t);
        void set_aggregated_end_time(time_t);
        void set_aggregated_profile(vector<TimeSlice>);
        void set_scheduled_allocation(double[24]);
        void set_duration(int); 
        void set_individual_offers(vector<Flexoffer>);

        //Utils
        void pretty_print();
        void schedule();
};

#endif
