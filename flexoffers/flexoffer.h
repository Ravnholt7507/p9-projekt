#ifndef FLEXOFFER_H
#define FLEXOFFER_H
#include <ctime>
#include <vector>
using namespace std;


struct TimeSlice {
    double min_power; // Minimum power in kW
    double max_power; // Maximum power in kW
};

class Flexoffer{
    public:
        Flexoffer(int oi, time_t est, time_t lst, time_t et, vector<TimeSlice> &p, int d);
        void print_flexoffer();
        int offer_id;
        time_t earliest_start_time;
        time_t latest_start_time;
        time_t end_time;
        vector<TimeSlice> profile;
        int duration; 
        double scheduled_allocation[24] = {0};
        time_t scheduled_start_time;
};

class dependencyFlexoffer{
    public:
        dependencyFlexoffer(int num_intervals);
        int num_intervals;
        vector<TimeSlice> dependency_bounds;
        vector<TimeSlice> energy_bounds;
        void pretty_print();
};

Flexoffer generateFlexOffer(int id);
vector<Flexoffer> generateMultipleFlexOffers(int num);
dependencyFlexoffer generateDFO();
#endif