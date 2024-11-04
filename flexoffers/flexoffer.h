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
        int earliest_start_time_asInt;
        int latest_start_time_asInt;
        int end_time_asInt;
        vector<TimeSlice> profile;
        int duration; 
        double scheduled_allocation[24] = {0};
        time_t scheduled_start_time;
};

Flexoffer generateFlexOffer(int id);
vector<Flexoffer> generateMultipleFlexOffers(int num);
#endif