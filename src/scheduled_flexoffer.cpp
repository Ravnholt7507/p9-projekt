#include <ctime>
#include <vector>
#include <iostream>
#include <iomanip>

#include "../include/scheduled_flexoffer.h"
#include "../include/flexoffer.h"
#include "../include/aggregation.h"
using namespace std;

//Constructor
ScheduledFlexOffer::ScheduledFlexOffer(AggregatedFlexOffer& AFO)
    : aggregated_offer(AFO){

    scheduled_profile.resize(24);
    for (int i = 0; i < 24; i++) {
        scheduled_profile[i] = (aggregated_offer.get_aggregated_profile()[i].min_power + 
                                aggregated_offer.get_aggregated_profile()[i].max_power) / 2.0;
    }
}

//Getters
AggregatedFlexOffer& ScheduledFlexOffer::get_aggregated_offer() {return aggregated_offer;};
int ScheduledFlexOffer::get_offer_id() {return offer_id;};
vector<double> ScheduledFlexOffer::get_scheduled_profile() {return scheduled_profile;};

//Setters
void ScheduledFlexOffer::set_aggregated_offer(AggregatedFlexOffer& value) {aggregated_offer = value;};
void ScheduledFlexOffer::set_offer_id(int value) {offer_id = value;};
void ScheduledFlexOffer::set_scheduled_profile(vector<double> value) {scheduled_profile = value;};


void ScheduledFlexOffer::print_schedule() {
    std::cout << "Scheduled allocation: " << std::endl;
    for (size_t i = 0; i < aggregated_offer.get_aggregated_profile().size(); i++) {
        if (scheduled_profile[i] > 0){
            std::cout << "  Hour " << i << ": Scheduled Power = " 
                    << fixed << setprecision(2) << scheduled_profile[i] << " kW" << endl;
        }
    }
};


// Implementing 1-To-N Disaggregation without the profile alignment vector
void ScheduledFlexOffer::n_to_1_disaggregation(vector<Flexoffer> &F, AggregatedFlexOffer &fa) {
    // Step 1: Loop through each profile element of the aggregated flex-offer (fa)
    double s_min[24] = {0};
    double s_max[24]= {0};
    double sx[24] = {0};

    for (size_t i = 0; i < fa.get_aggregated_profile().size(); i++) {
        s_min[i] = fa.get_aggregated_profile()[i].min_power;
        s_max[i] = fa.get_aggregated_profile()[i].max_power;
        sx[i] = scheduled_profile[i];

        // Calculate the new min and max power values for each individual flex-offer
        sx[i] = (s_max - s_min) != 0 ? ((sx - s_min) / (s_max - s_min)) : 0;
    }

    // Step 2: Loop through each flex-offer F to adjust the times (this here only works for start alignment, but can be changed easily) and calculate actual enery 
    vector<double> allocations{}; 
    for (size_t i = 0; i < F.size(); ++i) { // for each flexOffer
        allocations.clear();
        F[i].set_scheduled_start_time(F[i].get_est() / 3600);
        for (int j = 0; j < F[i].get_duration(); j++) { // for each slice
            allocations.push_back(F[i].get_profile()[j].min_power + ((F[i].get_profile()[j].max_power - F[i].get_profile()[j].min_power) * sx[i]));
        }
    }

    // Print disaggregated flex-offers
    cout << "=== Disaggregated FlexOffers ===" << std::endl;
    for (auto &f : F) {
        f.print_flexoffer();
    }
}
