#ifndef TEC_H
#define TEC_H
#include <vector>

#include "flexoffer.h"

class Tec_flexoffer : public Flexoffer {
    private:
        double min_overall_kw;
        double max_overall_kw; 
    public:
        //Constructor
        Tec_flexoffer(double min, double max, int oi, time_t est, time_t lst, time_t et, vector<TimeSlice> &p, int d);
        //Getters
        double get_min_overall_kw() const;
        double get_max_overall_kw() const;

        //Setters
        void set_min_overall_kw(double);
        void set_max_overall_kw(double);

        //overrides
        void print_flexoffer() override;
};
#endif
