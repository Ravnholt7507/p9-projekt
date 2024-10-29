#ifndef FLEXOFFER_H
#define FLEXOFFER_H
class Flexoffer{
    int offer_id;
    int earliest_start_time;
    int latest_start_time;
    double profile[24] = {0};
    int duration; 
    int end_time;
    
    public:
        Flexoffer(int oi, int est, int lst, double *p, int d, int et);
        void print_flexoffer();

};

#endif
