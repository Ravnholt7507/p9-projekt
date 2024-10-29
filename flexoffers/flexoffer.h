#ifndef FLEXOFFER_H
#define FLEXOFFER_H
#include <ctime>

struct TimeSlice {
    double min_power; // Minimum power in kW
    double max_power; // Maximum power in kW
};

class Flexoffer{
    int offer_id;
    time_t earliest_start_time;
    time_t end_time;
    TimeSlice profile[24] = {0};
    int duration; 
    
    public:
        Flexoffer(int oi, time_t est, time_t let, TimeSlice *p, int d);
        void print_flexoffer();

};

Flexoffer generateFlexOffer(int id);
#endif
