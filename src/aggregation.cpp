#include <iostream>
#include <ctime>
#include <iomanip>
#include <vector>
#include <cmath>

#include "aggregation.h"
#include "flexoffer.h"

using namespace std;

static int time_to_hour(time_t t) {
    struct tm * timeinfo = localtime(&t);
    return timeinfo->tm_hour;
}

// Constructor
AggregatedFlexOffer::AggregatedFlexOffer(int offer_id, const std::vector<Flexoffer> &offers) {
    if (offers.empty()) {
        throw invalid_argument("No FlexOffers provided for aggregation.");
    }

    id = offer_id;

    aggregated_earliest = std::numeric_limits<time_t>::max();
    aggregated_latest = 0;
    aggregated_end_time = 0;

    // Determine earliest, latest, and end times
    for (const auto& offer : offers) {
        aggregated_earliest = std::min(aggregated_earliest, offer.get_est());
        aggregated_latest = std::max(aggregated_latest, offer.get_lst());
        aggregated_end_time = std::max(aggregated_end_time, offer.get_et());
    }

    double total_seconds = difftime(aggregated_end_time, aggregated_earliest);
    duration = static_cast<int>(std::ceil(total_seconds / 3600.0));

    aggregated_profile.resize(duration, TimeSlice{0.0, 0.0});

    // Aggregate profiles
    for (const auto& offer : offers) {
        time_t est = offer.get_est();
        double diff_sec = difftime(est, aggregated_earliest);
        int start_hour = static_cast<int>(std::floor(diff_sec / 3600.0));

        for (int h = 0; h < offer.get_duration(); ++h) {
            int idx = start_hour + h;
            if (idx >= 0 && idx < duration) {
                aggregated_profile[idx].min_power += offer.get_profile()[h].min_power;
                aggregated_profile[idx].max_power += offer.get_profile()[h].max_power;
            }
        }
    }

    scheduled_allocation.resize(duration, 0.0);
    individual_offers = offers;
}

// Getters
int AggregatedFlexOffer::get_id() const { return id; }
time_t AggregatedFlexOffer::get_aggregated_earliest() const { return aggregated_earliest; }
time_t AggregatedFlexOffer::get_aggregated_latest() const { return aggregated_latest; }
time_t AggregatedFlexOffer::get_aggregated_end_time() const { return aggregated_end_time; }
std::vector<TimeSlice> AggregatedFlexOffer::get_aggregated_profile() const { return aggregated_profile; }
const std::vector<double>& AggregatedFlexOffer::get_scheduled_allocation() const { return scheduled_allocation; }
int AggregatedFlexOffer::get_duration() const { return duration; }
std::vector<Flexoffer> AggregatedFlexOffer::get_individual_offers() const { return individual_offers; }

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
void AggregatedFlexOffer::set_id(int value) { id = value; }
void AggregatedFlexOffer::set_aggregated_earliest(time_t value) { aggregated_earliest = value; }
void AggregatedFlexOffer::set_aggregated_latest(time_t value) { aggregated_latest = value; }
void AggregatedFlexOffer::set_aggregated_end_time(time_t value) { aggregated_end_time = value; }
void AggregatedFlexOffer::set_aggregated_profile(const std::vector<TimeSlice>& value) { aggregated_profile = value; scheduled_allocation.resize(value.size(),0.0); duration = (int)value.size(); }
void AggregatedFlexOffer::set_scheduled_allocation(const std::vector<double>& value) { 
    scheduled_allocation.clear();
    scheduled_allocation.insert(scheduled_allocation.end(), value.begin(), value.begin() + std::min(value.size(), (size_t)duration));
}
void AggregatedFlexOffer::set_duration(int value) { duration = value; scheduled_allocation.resize(value,0.0); }
void AggregatedFlexOffer::set_individual_offers(const std::vector<Flexoffer>& value) { individual_offers = value; }

// Utils
void AggregatedFlexOffer::pretty_print() const {
    // Helper lambda to convert time_t to readable format
    auto to_readable = [](time_t timestamp) -> string {
        char buffer[20];
        struct tm * timeinfo = localtime(&timestamp);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return string(buffer);
    };

    cout << "=== Aggregated FlexOffer Details ===" << endl;
    cout << "Offer ID: " << id << endl;
    cout << "Earliest Start Time: " << to_readable(aggregated_earliest) << endl;
    cout << "Latest Start Time:   " << to_readable(aggregated_latest) << endl;
    cout << "Latest End Time:     " << to_readable(aggregated_end_time) << endl;
    cout << "Duration:            " << duration << " hour(s)" << endl;
    cout << "Profile Elements:" << endl;

    for(int i = 0; i < duration; i++) {
        if(aggregated_profile[i].min_power > 0.0 || aggregated_profile[i].max_power > 0.0) {
            cout << "  Hour " << i << ": Min Power = " << fixed << setprecision(2) 
                 << aggregated_profile[i].min_power << " kW, Max Power = " 
                 << aggregated_profile[i].max_power << " kW" << endl;
        }
    }

    cout << "Scheduled Allocation:" << endl;
    for(int i = 0; i < duration; i++) {
        if(aggregated_profile[i].min_power > 0 || aggregated_profile[i].max_power > 0) {
            cout << "  Hour " << i << ": Scheduled Power = " << fixed << setprecision(2) 
                 << scheduled_allocation[i] << " kW" << endl;
        }
    }

    cout << "==============================" << endl;
}

void AggregatedFlexOffer::apply_schedule(const std::vector<double>& allocations) {
    size_t n = std::min(allocations.size(), static_cast<size_t>(duration));
    for (size_t i = 0; i < n; i++) {
        scheduled_allocation[i] = allocations[i];
    }
}

// Method to pad the profile to a given duration
void AggregatedFlexOffer::pad_profile(int target_duration) {
    if (duration >= target_duration) {
        return; // No padding needed
    }

    int slices_to_add = target_duration - duration;

    for (int i = 0; i < slices_to_add; ++i) {
        aggregated_profile.push_back({0.0, 0.0});
        scheduled_allocation.push_back(0.0);
    }

    duration = target_duration;
}
