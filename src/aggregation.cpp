#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <ctime>
#include <vector>
#include <variant>

#include "../include/aggregation.h"
#include "../include/flexoffer.h"
#include "../include/alignments.h"

using namespace std;

static int time_to_hour(time_t t) {
    struct tm * timeinfo = localtime(&t);
    return timeinfo->tm_hour;
}

//Constructors
AggregatedFlexOffer::AggregatedFlexOffer(int offer_id, const Alignments align, const vector<Flexoffer> &offers) {
    if (offers.empty()) {
        throw invalid_argument("No FlexOffers provided for aggregation.");
    }

    id = offer_id;

    cout << "=== Aggregating FlexOffers for AFO ID " << id << " ===" << endl;

    if(align == Alignments::start){
        start_alignment(aggregated_earliest, aggregated_latest, aggregated_end_time, aggregated_profile, duration, offers);
    } else if (align == Alignments::balance){
        balance_alignment(aggregated_earliest, aggregated_latest, aggregated_end_time, aggregated_profile, duration, offers);
    }

    scheduled_allocation.resize(duration, 0.0);
    for(auto offer : offers){
        individual_offers.push_back(offer);
    } 
}

AggregatedFlexOffer::AggregatedFlexOffer(int offer_id, const Alignments align, const vector<Tec_flexoffer> &offers) {
    if (offers.empty()) {
        throw invalid_argument("No FlexOffers provided for aggregation.");
    }

    id = offer_id;

    cout << "=== Aggregating FlexOffers for AFO ID " << id << " ===" << endl;
    // Determine aggregated earliest, latest start, and end times
    aggregated_earliest = numeric_limits<time_t>::max();
    aggregated_latest = 0;
    aggregated_end_time = 0;

    for (const auto &fo : offers) {
        aggregated_earliest = min(aggregated_earliest, fo.get_est());
        aggregated_latest = max(aggregated_latest, fo.get_lst());
        aggregated_end_time = max(aggregated_end_time, fo.get_et());
    }

    // Duration: from aggregated_latest to aggregated_end_time
    double diff_sec = difftime(aggregated_end_time, aggregated_latest);
    duration = (int) ceil(diff_sec / 3600.0);

    aggregated_profile.resize(duration, TimeSlice{0.0, 0.0});

    // Aggregate profiles: hour 0 in aggregated profile = aggregated_latest
    for (const auto &fo : offers) {
        // Align FO's LST with aggregated_latest
        double start_diff_sec = difftime(fo.get_lst(), aggregated_latest);
        int start_hour = (int) floor(start_diff_sec / 3600.0);

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
        overall_min += offer.get_min_overall_kw();
        overall_max += offer.get_max_overall_kw();
        individual_offers.push_back(offer);
    }
}

// Getters
int AggregatedFlexOffer::get_id() const { return id; }
time_t AggregatedFlexOffer::get_aggregated_earliest() const { return aggregated_earliest; }
time_t AggregatedFlexOffer::get_aggregated_latest() const { return aggregated_latest; }
time_t AggregatedFlexOffer::get_aggregated_end_time() const { return aggregated_end_time; }
vector<TimeSlice> AggregatedFlexOffer::get_aggregated_profile() const { return aggregated_profile; }
const vector<double>& AggregatedFlexOffer::get_scheduled_allocation() const { return scheduled_allocation; }
int AggregatedFlexOffer::get_duration() const { return duration; }
vector<variant<Flexoffer, Tec_flexoffer>> AggregatedFlexOffer::get_individual_offers() const {return individual_offers;}
double AggregatedFlexOffer::get_min_overall() const {return overall_min;}
double AggregatedFlexOffer::get_max_overall() const {return overall_max;}


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
void AggregatedFlexOffer::set_scheduled_allocation(const vector<double>& alloc) {
    for (size_t i = 0; i < alloc.size() && i < scheduled_allocation.size(); i++) {
        scheduled_allocation[i] = alloc[i];
    }
}
void AggregatedFlexOffer::set_id(int value) { id = value; }
void AggregatedFlexOffer::set_aggregated_earliest(time_t value) { aggregated_earliest = value; }
void AggregatedFlexOffer::set_aggregated_latest(time_t value) { aggregated_latest = value; }
void AggregatedFlexOffer::set_aggregated_end_time(time_t value) { aggregated_end_time = value; }
void AggregatedFlexOffer::set_aggregated_profile(const vector<TimeSlice>& value) { aggregated_profile = value; scheduled_allocation.resize(value.size(),0.0); duration = (int)value.size(); }
void AggregatedFlexOffer::set_duration(int value) { duration = value; scheduled_allocation.resize(value,0.0); }
void AggregatedFlexOffer::set_individual_offers(const vector<variant<Flexoffer, Tec_flexoffer>>& value) {
    for(auto offer : value){
        individual_offers.push_back(offer);
    }
}


// Utility
void AggregatedFlexOffer::apply_schedule(const vector<double>& allocations) {
    set_scheduled_allocation(allocations);
}

void AggregatedFlexOffer::pretty_print() const {
    auto to_readable = [](time_t t) {
        char buffer[20];
        struct tm * timeinfo = localtime(&t);
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

vector<Flexoffer> AggregatedFlexOffer::disaggregate_to_flexoffers() {
    
    // Compute fraction of allocation for each aggregated timeslice
    std::vector<double> fraction(duration, 0.0);
    for (int i = 0; i < duration; i++) {
        double denom = aggregated_profile[i].max_power - aggregated_profile[i].min_power;
        fraction[i] = (scheduled_allocation[i] - aggregated_profile[i].min_power) / denom;
        if (fraction[i] < 0.0) fraction[i] = 0.0;
        if (fraction[i] > 1.0) fraction[i] = 1.0;
    }

    // Disaggregate to each original Flexoffer
    std::vector<Flexoffer> result;

    time_t aggregated_schedule_start = aggregated_earliest;

    for (auto &vof : individual_offers) {
        if (holds_alternative<Flexoffer>(vof)) {
            Flexoffer f = get<Flexoffer>(vof);

            double start_diff_sec = difftime(f.get_lst(), aggregated_latest);
            int start_hour = (int)std::floor(start_diff_sec / 3600.0);

            std::vector<double> f_scheduled_allocation((size_t)f.get_duration(), 0.0);

            auto f_profile = f.get_profile();
            for (int h = 0; h < f.get_duration(); h++) {
                int idx = start_hour + h;
                if (idx >= 0 && idx < duration) {
                    double f_min = f_profile[h].min_power;
                    double f_max = f_profile[h].max_power;
                    double denom = f_max - f_min;
                    f_scheduled_allocation[h] = f_min + denom * fraction[idx];
                }
            }

            time_t f_scheduled_start = aggregated_schedule_start + (start_hour * 3600);

            f.set_scheduled_allocation(f_scheduled_allocation);
            f.set_scheduled_start_time(f_scheduled_start);

            result.push_back(f);
        }
    }

    return result;
}

vector<Tec_flexoffer> AggregatedFlexOffer::disaggregate_to_flexoffers_tec() {
}