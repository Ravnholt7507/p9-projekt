#include <cmath>
#include <iostream>
#include <algorithm>

#include "../include/alignments.h"

using namespace std;

Flexoffer least_flexible_object(vector<Flexoffer>&);
void calc_alignment(vector<TimeSlice>&, Flexoffer, int, double&, time_t&, time_t&, time_t&, int&);

void start_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time,
                     vector<TimeSlice> &aggregated_profile, int &duration, const vector<Flexoffer> &offers){

    aggregated_earliest = numeric_limits<time_t>::max();
    aggregated_latest = 0;
    aggregated_end_time = 0;

    for (const auto &fo : offers) {
        aggregated_earliest = min(aggregated_earliest, fo.get_est());
        aggregated_latest = max(aggregated_latest, fo.get_lst());
        aggregated_end_time = max(aggregated_end_time, fo.get_et());
    }

    double diff_sec = difftime(aggregated_end_time, aggregated_latest);
    duration = (int) ceil(diff_sec / 3600.0);

    aggregated_profile.resize(duration, TimeSlice{0.0, 0.0});

    for (const auto &fo : offers) {
        double start_diff_sec = difftime(fo.get_lst(), aggregated_latest);
        int start_hour = (int) floor(start_diff_sec / 3600.0);

        cout << "  FlexOffer ID " << fo.get_offer_id() << " alignment: start_hour = " << start_hour << endl;

        auto p = fo.get_profile();
        for (int h = 0; h < fo.get_duration(); h++) {
            int idx = start_hour + h;
            if (idx >= 0 && idx < duration) {
                aggregated_profile[idx].min_power += p[h].min_power;
                aggregated_profile[idx].max_power += p[h].max_power;
            }
        }
    }
}

void balance_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time,
                       vector<TimeSlice> &aggregated_profile, int &duration, vector<Flexoffer> offers){

    Flexoffer least_flexible = least_flexible_object(offers);  
    aggregated_earliest = least_flexible.get_est();
    aggregated_latest = least_flexible.get_lst();
    aggregated_end_time = least_flexible.get_et();
    aggregated_profile = least_flexible.get_profile();
    double diff_sec = difftime(aggregated_end_time, aggregated_latest);
    duration = (int) ceil(diff_sec / 3600.0);

    while(offers.size() > 0){
        least_flexible = least_flexible_object(offers);    
        if(aggregated_latest < least_flexible.get_est()){
            double diff_sec = difftime(least_flexible.get_est(), aggregated_latest);
            int empty_space = (int) ceil(diff_sec / 3600.0);
            for(int i = 0; i < empty_space; i++) aggregated_profile.push_back({0,0});
            for(auto slice : least_flexible.get_profile()) aggregated_profile.push_back(slice);
            aggregated_latest = least_flexible.get_lst(); 
            aggregated_end_time += (least_flexible.get_duration()*3600); //Er det her rigtigt??
            duration = aggregated_profile.size();
        } else if(aggregated_earliest > least_flexible.get_lst()){
            double diff_sec = difftime(aggregated_earliest, least_flexible.get_lst());
            int empty_space = (int) ceil(diff_sec / 3600.0);
            for(int i = 0; i < empty_space; i++) aggregated_profile.insert(aggregated_profile.begin(), {0,0});
            vector<TimeSlice> tmp = least_flexible.get_profile();
            reverse(tmp.begin(), tmp.begin());
            for(auto slice : tmp) aggregated_profile.insert(aggregated_profile.begin(), slice);
            aggregated_earliest = least_flexible.get_lst();
            duration = aggregated_profile.size();
        } else {
            double offsetMinSec = difftime(least_flexible.get_est(), aggregated_earliest);
            double offsetMaxSec = difftime(least_flexible.get_lst(), aggregated_earliest);
            int offsetMin = floor(offsetMinSec / 3600.0);
            int offsetMax = floor(offsetMaxSec / 3600.0);
            for(int i = offsetMin; i < offsetMax; i++){
                double best_result = 0;
                calc_alignment(aggregated_profile, least_flexible, i, best_result, aggregated_earliest, aggregated_latest, aggregated_end_time, duration);
            }
        }
    }
}

void calc_alignment(vector<TimeSlice> &aggregated_profile, Flexoffer least_flexible, int offset, double &best_result,
                    time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, int &duration){
    vector<TimeSlice> tmp{};    
    vector<TimeSlice> least_profile = least_flexible.get_profile();
    int i{offset};
    while(i < 0 && !least_profile.empty()){
        tmp.push_back(least_profile[0]); 
        least_profile.erase(least_profile.begin());
        i++;
    }
    while(i > 0 && !aggregated_profile.empty()){
        tmp.push_back(aggregated_profile[0]);
        aggregated_profile.erase(aggregated_profile.begin()); 
        i--;
    }
    while(!least_profile.empty() && !aggregated_profile.empty()){
        tmp.push_back({aggregated_profile[0].min_power + least_profile[0].min_power, aggregated_profile[0].max_power + least_profile[0].max_power}); 
        least_profile.erase(least_profile.begin());
        aggregated_profile.erase(aggregated_profile.begin());
    }
    while(!least_profile.empty()){
        tmp.push_back(least_profile[0]); 
        least_profile.erase(least_profile.begin());
    }
    while(!aggregated_profile.empty()){
        tmp.push_back(aggregated_profile[0]);
        aggregated_profile.erase(aggregated_profile.begin()); 
    }

    double result{0};
    for(auto& element : tmp){
        result += element.max_power - element.min_power;
    } 
    result /= tmp.size();

    if(result > best_result){
        best_result = result;
        aggregated_earliest = min(aggregated_earliest, least_flexible.get_est());
        aggregated_latest = max(aggregated_latest, least_flexible.get_lst());
        duration = tmp.size();
        aggregated_end_time = aggregated_latest + (duration*3600);
        aggregated_profile.clear();
        aggregated_profile = tmp;
    }
}

Flexoffer least_flexible_object(vector<Flexoffer> &offers){
    double avg_flex{0};
    double min_flex{numeric_limits<double>::max()};
    size_t id{0};

    for(size_t i = 0; i < offers.size(); i++){
        avg_flex = 0; 

        for(const auto& slice : offers[i].get_profile()){
            avg_flex += slice.max_power/2; 
        }        

        if(avg_flex < min_flex){
            min_flex = avg_flex;
            id = i;
        }
    }
    Flexoffer least_flexible = offers[id];  
    offers.erase(next(offers.begin(), id));

    return least_flexible;
}