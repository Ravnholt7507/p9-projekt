#ifndef SCHEDULED_FLEX_OFFER_H
#define SCHEDULED_FLEX_OFFER_H
#include <ctime>
#include <vector>

#include "flexoffer.h"
#include "aggregation.h"

using namespace std;


class ScheduledFlexOffer {
    private:
        AggregatedFlexOffer &aggregated_offer;
        int offer_id;
        vector<double> scheduled_profile;
    public:
        //Constructor
        ScheduledFlexOffer(AggregatedFlexOffer &AFO);

        //Getters
        AggregatedFlexOffer& get_aggregated_offer();
        int get_offer_id();
        vector<double> get_scheduled_profile();

        //Setters
        void set_aggregated_offer(AggregatedFlexOffer&);
        void set_offer_id(int);
        void set_scheduled_profile(vector<double>);

        //Utils
        void schedule();
        void print_schedule();
        void n_to_1_disaggregation(vector<Flexoffer> &F, AggregatedFlexOffer &fa);
};

#endif // SCHEDULED_FLEX_OFFER_H
