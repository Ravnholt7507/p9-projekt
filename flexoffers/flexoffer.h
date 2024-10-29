#ifndef FLEXOFFER_H
#define FLEXOFFER_H
#include <ctime>
class Flexoffer{
    int offer_id;
    time_t earliest_start_time;
    time_t latest_start_time;
    double profile[24] = {0};
    int duration; 
    time_t end_time;
    
    public:
        Flexoffer(int oi, time_t est, time_t lst, double *p, int d, time_t et);
        void print_flexoffer();

};

Flexoffer generateFlexOffer(int id);
#endif
