#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <ctime>
#include <vector>
#include <variant>

#include "../include/aggregation.h"
#include "../include/flexoffer.h"

using namespace std;


static int time_to_hour(time_t t) {
    struct tm * timeinfo = localtime(&t);
    return timeinfo->tm_hour;
}

//Constructors
AggregatedFlexOffer::AggregatedFlexOffer(int offer_id, const std::vector<Flexoffer> &offers) {
    if (offers.empty()) {
        throw std::invalid_argument("No FlexOffers provided for aggregation.");
    }

    id = offer_id;

    cout << "=== Aggregating FlexOffers for AFO ID " << id << " ===" << endl;
    // Determine aggregated earliest, latest start, and end times
    aggregated_earliest = std::numeric_limits<time_t>::max();
    aggregated_latest = 0;
    aggregated_end_time = 0;

    for (const auto &fo : offers) {
        aggregated_earliest = std::min(aggregated_earliest, fo.get_est());
        aggregated_latest = std::max(aggregated_latest, fo.get_lst());
        aggregated_end_time = std::max(aggregated_end_time, fo.get_et());
    }

    // Duration: from aggregated_latest to aggregated_end_time
    double diff_sec = difftime(aggregated_end_time, aggregated_latest);
    duration = (int) std::ceil(diff_sec / 3600.0);

    aggregated_profile.resize(duration, TimeSlice{0.0, 0.0});

    // Aggregate profiles: hour 0 in aggregated profile = aggregated_latest
    for (const auto &fo : offers) {
        // Align FO's LST with aggregated_latest
        double start_diff_sec = difftime(fo.get_lst(), aggregated_latest);
        int start_hour = (int) std::floor(start_diff_sec / 3600.0);

        cout << "  FlexOffer ID " << fo.get_offer_id() << " alignment: start_hour = " << start_hour << endl;

        auto p = fo.get_profile();
        for (int h = 0; h < fo.get_duration(); h++) {
            int idx = start_hour + h;
            if (idx >= 0 && idx < duration) {
                aggregated_profile[idx].min_power += p[h].min_power;
                aggregated_profile[idx].max_power += p[h].max_power;
            } else {
                // Out of range - skip
            }
        }
    }

    scheduled_allocation.resize(duration, 0.0);
    for(auto offer : offers){
        individual_offers.push_back(offer);
    } 
}

AggregatedFlexOffer::AggregatedFlexOffer(int offer_id, const std::vector<Tec_flexoffer> &offers) {
    if (offers.empty()) {
        throw std::invalid_argument("No FlexOffers provided for aggregation.");
    }

    id = offer_id;

    cout << "=== Aggregating FlexOffers for AFO ID " << id << " ===" << endl;
    // Determine aggregated earliest, latest start, and end times
    aggregated_earliest = std::numeric_limits<time_t>::max();
    aggregated_latest = 0;
    aggregated_end_time = 0;

    for (const auto &fo : offers) {
        aggregated_earliest = std::min(aggregated_earliest, fo.get_est());
        aggregated_latest = std::max(aggregated_latest, fo.get_lst());
        aggregated_end_time = std::max(aggregated_end_time, fo.get_et());
    }

    // Duration: from aggregated_latest to aggregated_end_time
    double diff_sec = difftime(aggregated_end_time, aggregated_latest);
    duration = (int) std::ceil(diff_sec / 3600.0);

    aggregated_profile.resize(duration, TimeSlice{0.0, 0.0});

    // Aggregate profiles: hour 0 in aggregated profile = aggregated_latest
    for (const auto &fo : offers) {
        // Align FO's LST with aggregated_latest
        double start_diff_sec = difftime(fo.get_lst(), aggregated_latest);
        int start_hour = (int) std::floor(start_diff_sec / 3600.0);

        cout << "  FlexOffer ID " << fo.get_offer_id() << " alignment: start_hour = " << start_hour << endl;

        auto p = fo.get_profile();
        for (int h = 0; h < fo.get_duration(); h++) {
            int idx = start_hour + h;
            if (idx >= 0 && idx < duration) {
                aggregated_profile[idx].min_power += p[h].min_power;
                aggregated_profile[idx].max_power += p[h].max_power;
            } else {
                // Out of range - skip
            }
        }
    }

    scheduled_allocation.resize(duration, 0.0);
    for(auto offer : offers){
        individual_offers.push_back(offer);
    }
}

// Getters
int AggregatedFlexOffer::get_id() const { return id; }
time_t AggregatedFlexOffer::get_aggregated_earliest() const { return aggregated_earliest; }
time_t AggregatedFlexOffer::get_aggregated_latest() const { return aggregated_latest; }
time_t AggregatedFlexOffer::get_aggregated_end_time() const { return aggregated_end_time; }
std::vector<TimeSlice> AggregatedFlexOffer::get_aggregated_profile() const { return aggregated_profile; }
const std::vector<double>& AggregatedFlexOffer::get_scheduled_allocation() const { return scheduled_allocation; }
int AggregatedFlexOffer::get_duration() const { return duration; }
std::vector<variant<Flexoffer, Tec_flexoffer>> AggregatedFlexOffer::get_individual_offers() const {return individual_offers;}


// Hour-based getters
int AggregatedFlexOffer::get_aggregated_earliest_hour() const {
    return time_to_hour(aggregated_earliest);
}
int AggregatedFlexOffer::get_aggregated_latest_hour() const {
    return time_to_hour(aggregated_latest);
}
int AggregatedFlexOffer::get_aggregated_end_time_hour() const {
    return time_to_hour(aggregated_end_time);
}

// Setters
void AggregatedFlexOffer::set_scheduled_allocation(const std::vector<double>& alloc) {
    for (size_t i = 0; i < alloc.size() && i < scheduled_allocation.size(); i++) {
        scheduled_allocation[i] = alloc[i];
    }
}
void AggregatedFlexOffer::set_id(int value) { id = value; }
void AggregatedFlexOffer::set_aggregated_earliest(time_t value) { aggregated_earliest = value; }
void AggregatedFlexOffer::set_aggregated_latest(time_t value) { aggregated_latest = value; }
void AggregatedFlexOffer::set_aggregated_end_time(time_t value) { aggregated_end_time = value; }
void AggregatedFlexOffer::set_aggregated_profile(const std::vector<TimeSlice>& value) { aggregated_profile = value; scheduled_allocation.resize(value.size(),0.0); duration = (int)value.size(); }
void AggregatedFlexOffer::set_duration(int value) { duration = value; scheduled_allocation.resize(value,0.0); }
void AggregatedFlexOffer::set_individual_offers(const std::vector<variant<Flexoffer, Tec_flexoffer>>& value) {
    for(auto offer : value){
        individual_offers.push_back(offer);
    }
}


// Utility
void AggregatedFlexOffer::apply_schedule(const std::vector<double>& allocations) {
    set_scheduled_allocation(allocations);
}

void AggregatedFlexOffer::pretty_print() const {
    auto to_readable = [](time_t t) {
        char buffer[20];
        struct tm * timeinfo = localtime(&t);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    };

    cout << "=== Aggregated FlexOffer Details ===" << endl;
    cout << "Offer ID: " << id << endl;
    cout << "Earliest Start Time: " << to_readable(aggregated_earliest) << endl;
    cout << "Latest Start Time:   " << to_readable(aggregated_latest) << endl;
    cout << "Latest End Time:     " << to_readable(aggregated_end_time) << endl;
    cout << "Duration:            " << duration << " hour(s)" << endl;

    cout << "Profile Elements:" << endl;
    for (int i = 0; i < duration; i++) {
        time_t t = aggregated_latest + i * 3600;
        cout << "  Hour " << i << " (" << to_readable(t) << "): "
             << "Min=" << aggregated_profile[i].min_power << " kW, "
             << "Max=" << aggregated_profile[i].max_power << " kW" << endl;
    }

    cout << "Scheduled Allocation:" << endl;
    for (int i = 0; i < duration; i++) {
        time_t t = aggregated_latest + i * 3600;
        cout << "  Hour " << i << " (" << to_readable(t) << "): "
             << "Power=" << scheduled_allocation[i] << " kW" << endl;
    }
    cout << "==============================" << endl;
}
