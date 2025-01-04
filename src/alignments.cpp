#include <cmath>
#include <iostream>
#include <algorithm>

#include "../include/alignments.h"

using namespace std;

Flexoffer least_flexible_object(vector<Flexoffer>&);
Tec_flexoffer least_flexible_object(vector<Tec_flexoffer> &offers);
vector<TimeSlice> calc_alignment(vector<TimeSlice>, Flexoffer, int, double&); 
vector<TimeSlice> calc_alignment(vector<TimeSlice>, Tec_flexoffer, int, double&);
vector<TimeSlice> calc_priceAwareAlignment(vector<TimeSlice> &aggregated_profile, const Flexoffer &least_flexible,int offset,double &best_synergy, const vector<double> &spotPrices);
vector<TimeSlice> calc_priceAwareAlignment(vector<TimeSlice> &aggregated_profile,const Tec_flexoffer &least_flexible,int offset,double &best_synergy, const vector<double> &spotPrices);

//for fo
void start_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, vector<TimeSlice> &aggregated_profile, int &duration, vector<Flexoffer> offers){
    aggregated_earliest = offers[0].get_est();
    aggregated_latest = offers[0].get_lst();
    aggregated_end_time = offers[0].get_et();

    vector<TimeSlice> tmp;
    for (const auto& fo : offers) {
        int i = aggregated_earliest - fo.get_est();
        if(i != 0) i /= 3600;
        vector<TimeSlice> foCopy = fo.get_profile();
        vector<TimeSlice> aggCopy = aggregated_profile;
        while(i < 0 && !aggCopy.empty()){
            tmp.push_back(aggCopy[0]);   
            aggCopy.erase(aggCopy.begin());
            i++;
        }
        while(i>0 && !foCopy.empty()){
            tmp.push_back(foCopy[0]);
            foCopy.erase(foCopy.begin());
            i--;
        }
        while(!aggCopy.empty() && !foCopy.empty()){
            tmp.push_back({aggCopy[0].min_power + foCopy[0].min_power, aggCopy[0].max_power + foCopy[0].max_power});
            aggCopy.erase(aggCopy.begin());
            foCopy.erase(foCopy.begin());
        }
        while(!foCopy.empty()){
            tmp.push_back(foCopy[0]);   
            foCopy.erase(foCopy.begin());
        }
        while(!aggCopy.empty()){
            tmp.push_back(aggCopy[0]);
            aggCopy.erase(aggCopy.begin());
        }

        duration = tmp.size();
        aggregated_earliest = min(aggregated_earliest, fo.get_est());
        aggregated_end_time = min(aggregated_end_time, fo.get_et());
        aggregated_latest = aggregated_end_time - (duration * 3600);
        aggregated_profile = tmp;
        tmp.clear();
    }
}

//for tec
void start_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, vector<TimeSlice> &aggregated_profile, int &duration, double &overall_min, double &overall_max, const vector<Tec_flexoffer> &offers){

    aggregated_earliest = numeric_limits<time_t>::max();
    aggregated_latest = 0;
    aggregated_end_time = 0;

    for (const auto &fo : offers) {
        aggregated_earliest = min(aggregated_earliest, fo.get_est());
        aggregated_latest = max(aggregated_latest, fo.get_lst());
        aggregated_end_time = max(aggregated_end_time, fo.get_et());
        overall_min += fo.get_min_overall_kw();
        overall_max += fo.get_max_overall_kw();
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

//for fo
void balance_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, vector<TimeSlice> &aggregated_profile, int &duration, vector<Flexoffer> offers){

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
            reverse(tmp.begin(), tmp.end());
            for(auto slice : tmp) aggregated_profile.insert(aggregated_profile.begin(), slice);
            aggregated_earliest = least_flexible.get_lst();
            duration = aggregated_profile.size();
        } else {
            double offsetMinSec = difftime(least_flexible.get_est(), aggregated_earliest);
            double offsetMaxSec = difftime(least_flexible.get_lst(), aggregated_earliest);
            int offsetMin = floor(offsetMinSec / 3600.0);
            int offsetMax = floor(offsetMaxSec / 3600.0);
            double best_result = 0;

            time_t best_earliest;
            time_t best_latest;
            int best_duration;
            time_t best_endtime;
            vector<TimeSlice> best_profile;
            for(int i = offsetMin; i <= offsetMax; i++){
                double placeholder = best_result;
                vector<TimeSlice> tmp = calc_alignment(aggregated_profile, least_flexible, i, best_result);
                if(best_result > placeholder){
                    best_earliest = aggregated_earliest + (i * 3600);
                    best_latest = min(aggregated_latest, least_flexible.get_lst());
                    best_duration = tmp.size();
                    best_endtime = aggregated_latest + ((duration-1)*3600);
                    best_profile = tmp;
                }
            }
            aggregated_earliest = best_earliest;
            aggregated_latest = best_latest;
            duration = best_duration;
            aggregated_end_time = best_endtime;
            aggregated_profile = best_profile;
        }
    }
}

//for tec
void balance_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, vector<TimeSlice> &aggregated_profile, int &duration, double &overall_min, double &overall_max, vector<Tec_flexoffer> offers){

    Tec_flexoffer least_flexible = least_flexible_object(offers);  
    aggregated_earliest = least_flexible.get_est();
    aggregated_latest = least_flexible.get_lst();
    aggregated_end_time = least_flexible.get_et();
    aggregated_profile = least_flexible.get_profile();
    overall_min += least_flexible.get_min_overall_kw();
    overall_max += least_flexible.get_max_overall_kw();
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
            overall_min += least_flexible.get_min_overall_kw();
            overall_max += least_flexible.get_max_overall_kw();
        } else if(aggregated_earliest > least_flexible.get_lst()){
            double diff_sec = difftime(aggregated_earliest, least_flexible.get_lst());
            int empty_space = (int) ceil(diff_sec / 3600.0);
            for(int i = 0; i < empty_space; i++) aggregated_profile.insert(aggregated_profile.begin(), {0,0});
            vector<TimeSlice> tmp = least_flexible.get_profile();
            reverse(tmp.begin(), tmp.end());
            for(auto slice : tmp) aggregated_profile.insert(aggregated_profile.begin(), slice);
            aggregated_earliest = least_flexible.get_lst();
            duration = aggregated_profile.size();
            overall_min += least_flexible.get_min_overall_kw();
            overall_max += least_flexible.get_max_overall_kw();
        } else {
            double offsetMinSec = difftime(least_flexible.get_est(), aggregated_earliest);
            double offsetMaxSec = difftime(least_flexible.get_lst(), aggregated_earliest);
            int offsetMin = floor(offsetMinSec / 3600.0);
            int offsetMax = floor(offsetMaxSec / 3600.0);


            time_t best_earliest;
            time_t best_latest;
            int best_duration;
            time_t best_endtime;
            vector<TimeSlice> best_profile;
            double best_result = 0;
            double best_min;
            double best_max;
            for(int i = offsetMin; i <= offsetMax; i++){
                double placeholder = best_result;
                vector<TimeSlice> tmp = calc_alignment(aggregated_profile, least_flexible, i, best_result);
                if(best_result > placeholder){
                    best_earliest = aggregated_earliest + (i * 3600);
                    best_latest = min(aggregated_latest, least_flexible.get_lst());
                    best_duration = tmp.size();
                    best_endtime = aggregated_latest + (duration*3600);
                    best_profile = tmp;
                    best_min = overall_min + least_flexible.get_min_overall_kw();
                    best_max = overall_max + least_flexible.get_max_overall_kw();
                }
            }
            aggregated_earliest = best_earliest;
            aggregated_latest = best_latest;
            duration = best_duration;
            aggregated_end_time = best_endtime;
            aggregated_profile = best_profile;
            overall_min = best_min;
            overall_max = best_max;
        }
    }
}

//for fo
vector<TimeSlice> calc_alignment(vector<TimeSlice> aggregated_profile, Flexoffer least_flexible, int offset, double &best_result){
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
    }
    return tmp;
}

//For tec
vector<TimeSlice> calc_alignment(vector<TimeSlice> aggregated_profile, Tec_flexoffer least_flexible, int offset, double &best_result){
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
    }
    return tmp;
}


//for Fo
Flexoffer least_flexible_object(vector<Flexoffer> &offers){
    double avg_flex{0};
    double min_flex{numeric_limits<double>::max()};
    size_t id{0};

    for(size_t i = 0; i < offers.size(); i++){
        avg_flex = 0; 

        for(const auto& slice : offers[i].get_profile()){
            avg_flex += slice.max_power; 
        }        
        avg_flex /= offers[i].get_profile().size();

        if(avg_flex <= min_flex){
            min_flex = avg_flex;
            id = i;
        }
    }
    Flexoffer least_flexible = offers[id];  
    offers.erase(next(offers.begin(), id));

    return least_flexible;
}

//for tec
Tec_flexoffer least_flexible_object(vector<Tec_flexoffer> &offers){
    double avg_flex{0};
    double min_flex{numeric_limits<double>::max()};
    size_t id{0};

    for(size_t i = 0; i < offers.size(); i++){
        avg_flex = 0; 

        for(const auto& slice : offers[i].get_profile()){
            avg_flex += slice.max_power; 
        }        
        avg_flex /= offers[i].get_profile().size();

        if(avg_flex < min_flex){
            min_flex = avg_flex;
            id = i;
        }
    }
    Tec_flexoffer least_flexible = offers[id];  
    offers.erase(next(offers.begin(), id));

    return least_flexible;
}

//For fo
void priceAwareAlignment(time_t &aggregated_earliest,time_t &aggregated_latest,time_t &aggregated_end_time,vector<TimeSlice> &aggregated_profile,int &duration,vector<Flexoffer> offers,const vector<double> &spotPrices){
    Flexoffer least_flexible = least_flexible_object(offers);

    aggregated_earliest = least_flexible.get_est();
    aggregated_latest = least_flexible.get_lst();
    aggregated_end_time = least_flexible.get_et();
    aggregated_profile = least_flexible.get_profile();

    double diff_sec = difftime(aggregated_end_time, aggregated_latest);
    duration = ceil(diff_sec / 3600.0);

    while(!offers.empty())
    {
        least_flexible = least_flexible_object(offers);

        if(aggregated_latest < least_flexible.get_est())
        {
            double gapSec = difftime(least_flexible.get_est(), aggregated_latest);
            int empty_space = ceil(gapSec / 3600.0);
            for(int i = 0; i < empty_space; ++i)
                aggregated_profile.push_back({0,0});
            
            for(auto &slice : least_flexible.get_profile())
                aggregated_profile.push_back(slice);

            aggregated_latest = least_flexible.get_lst();
            aggregated_end_time += (least_flexible.get_duration() * 3600);
            duration = aggregated_profile.size();
        }
        else if(aggregated_earliest > least_flexible.get_lst())
        {
            double gapSec = difftime(aggregated_earliest, least_flexible.get_lst());
            int empty_space = ceil(gapSec / 3600.0);
            for(int i=0; i<empty_space; ++i)
                aggregated_profile.insert(aggregated_profile.begin(), {0,0});

            vector<TimeSlice> tmp = least_flexible.get_profile();
            reverse(tmp.begin(), tmp.end()); 
            for(auto &slice : tmp)
                aggregated_profile.insert(aggregated_profile.begin(), slice);

            aggregated_earliest = least_flexible.get_lst();
            duration = aggregated_profile.size();
        }
        else
        {
            double offsetMinSec = difftime(least_flexible.get_est(), aggregated_earliest);
            double offsetMaxSec = difftime(least_flexible.get_lst(), aggregated_earliest);
            int offsetMin = floor(offsetMinSec / 3600.0);
            int offsetMax = floor(offsetMaxSec / 3600.0);

            time_t best_earliest;
            time_t best_latest;
            int best_duration;
            time_t best_endtime;
            vector<TimeSlice> best_profile;

            double best_synergy = 0;
            for(int i = offsetMin; i < offsetMax; i++)
            {
                double placeholder = best_synergy;
                vector<TimeSlice> tmp = calc_priceAwareAlignment(aggregated_profile, least_flexible, i, best_synergy, spotPrices);
                
                if(placeholder > best_synergy){
                    best_earliest = aggregated_earliest + (i * 3600);
                    best_latest = min(aggregated_latest, least_flexible.get_lst());
                    best_duration = tmp.size();
                    best_endtime = aggregated_latest + ((duration-1)*3600);
                    best_profile = tmp;
                }
            }
            aggregated_earliest = best_earliest;
            aggregated_latest = best_latest;
            duration = best_duration;
            aggregated_end_time = best_endtime;
            aggregated_profile = best_profile;
        }
    }
}


vector<TimeSlice> calc_priceAwareAlignment(vector<TimeSlice> &aggregated_profile,const Flexoffer &least_flexible,int offset,double &best_synergy, const vector<double> &spotPrices){
    vector<TimeSlice> tmp;
    vector<TimeSlice> fo_profile = least_flexible.get_profile();
    int i = offset;

    while(i < 0 && !fo_profile.empty()){
        tmp.push_back(fo_profile.front());
        fo_profile.erase(fo_profile.begin());
        i++;
    }

    while(i > 0 && !aggregated_profile.empty()){
        tmp.push_back(aggregated_profile.front());
        aggregated_profile.erase(aggregated_profile.begin());
        i--;
    }

    while(!fo_profile.empty() && !aggregated_profile.empty()){
        double newMin = aggregated_profile.front().min_power + fo_profile.front().min_power;
        double newMax = aggregated_profile.front().max_power + fo_profile.front().max_power;
        tmp.push_back({ newMin, newMax });
        aggregated_profile.erase(aggregated_profile.begin());
        fo_profile.erase(fo_profile.begin());
    }

    while(!fo_profile.empty()){
        tmp.push_back(fo_profile.front());
        fo_profile.erase(fo_profile.begin());
    }
    while(!aggregated_profile.empty()){
        tmp.push_back(aggregated_profile.front());
        aggregated_profile.erase(aggregated_profile.begin());
    }

    // Compute price synergy
    double synergy  = 0.0;
    double maxPrice = 0.0;
    for(double p : spotPrices){ //get max price
        if(p > maxPrice) maxPrice = p;
    }

    for(size_t h = 0; h < tmp.size(); h++){
        double avgLoad = (tmp[h].min_power + tmp[h].max_power)*0.5;
        int hourIndex = (offset + h);
        double localPrice = spotPrices[hourIndex];
        synergy += avgLoad * (maxPrice - localPrice);
    }

    if(synergy > best_synergy){
        best_synergy = synergy;
    }
    return tmp;
}

void priceAwareAlignment(time_t &aggregated_earliest,time_t &aggregated_latest,time_t &aggregated_end_time,vector<TimeSlice> &aggregated_profile,int &duration,double &overall_min,double &overall_max,vector<Tec_flexoffer> offers, const vector<double> &spotPrices){
    Tec_flexoffer least_flexible = least_flexible_object(offers);
    aggregated_earliest = least_flexible.get_est();
    aggregated_latest = least_flexible.get_lst();
    aggregated_end_time = least_flexible.get_et();
    aggregated_profile = least_flexible.get_profile();
    overall_min += least_flexible.get_min_overall_kw();
    overall_max += least_flexible.get_max_overall_kw();

    double diff_sec = difftime(aggregated_end_time, aggregated_latest);
    duration = ceil(diff_sec / 3600.0);

    while(!offers.empty())
    {
        least_flexible = least_flexible_object(offers);

        if(aggregated_latest < least_flexible.get_est())
        {
            double gapSec = difftime(least_flexible.get_est(), aggregated_latest);
            int empty_space = ceil(gapSec / 3600.0);
            for(int i=0; i<empty_space; i++)
                aggregated_profile.push_back({0,0});
            
            for(auto &sl : least_flexible.get_profile())
                aggregated_profile.push_back(sl);

            aggregated_latest = least_flexible.get_lst();
            aggregated_end_time += (least_flexible.get_duration()*3600);
            duration = aggregated_profile.size();
            overall_min += least_flexible.get_min_overall_kw();
            overall_max += least_flexible.get_max_overall_kw();
        }
        else if(aggregated_earliest > least_flexible.get_lst())
        {
            double gapSec = difftime(aggregated_earliest, least_flexible.get_lst());
            int empty_space = ceil(gapSec / 3600.0);
            for(int i=0; i<empty_space; i++)
                aggregated_profile.insert(aggregated_profile.begin(), {0,0});

            vector<TimeSlice> tmp = least_flexible.get_profile();
            reverse(tmp.begin(), tmp.end());
            for(auto &sl : tmp)
                aggregated_profile.insert(aggregated_profile.begin(), sl);

            aggregated_earliest = least_flexible.get_lst();
            duration = aggregated_profile.size();
            overall_min += least_flexible.get_min_overall_kw();
            overall_max += least_flexible.get_max_overall_kw();
        }
        else
        {
            double offsetMinSec = difftime(least_flexible.get_est(), aggregated_earliest);
            double offsetMaxSec = difftime(least_flexible.get_lst(), aggregated_earliest);
            int offsetMin = floor(offsetMinSec / 3600.0);
            int offsetMax = floor(offsetMaxSec / 3600.0);

            time_t best_earliest;
            time_t best_latest;
            int best_duration;
            time_t best_endtime;
            vector<TimeSlice> best_profile;
            double best_min;
            double best_max;

            double best_synergy = 0;
            for(int i = offsetMin; i < offsetMax; i++)
            {
                double placeholder = best_synergy;
                vector<TimeSlice> tmp = calc_priceAwareAlignment(aggregated_profile, least_flexible, i, best_synergy, spotPrices);
                
                if(placeholder > best_synergy){
                    best_earliest = aggregated_earliest + (i * 3600);
                    best_latest = min(aggregated_latest, least_flexible.get_lst());
                    best_duration = tmp.size();
                    best_endtime = aggregated_latest + ((duration-1)*3600);
                    best_profile = tmp;
                    best_min = overall_min + least_flexible.get_min_overall_kw();
                    best_max = overall_max + least_flexible.get_max_overall_kw();
                }
            }
            aggregated_earliest = best_earliest;
            aggregated_latest = best_latest;
            duration = best_duration;
            aggregated_end_time = best_endtime;
            aggregated_profile = best_profile;
            overall_min = best_min;
            overall_max = best_max;
        }
    }
}

vector<TimeSlice> calc_priceAwareAlignment(vector<TimeSlice> &aggregated_profile, const Tec_flexoffer &least_flexible, int offset, double &best_synergy, const vector<double> &spotPrices){
    vector<TimeSlice> tmp;
    vector<TimeSlice> fo_profile = least_flexible.get_profile();
    int i = offset;

    while(i < 0 && !fo_profile.empty()){
        tmp.push_back(fo_profile.front());
        fo_profile.erase(fo_profile.begin());
        i++;
    }
    while(i > 0 && !aggregated_profile.empty()){
        tmp.push_back(aggregated_profile.front());
        aggregated_profile.erase(aggregated_profile.begin());
        i--;
    }
    while(!fo_profile.empty() && !aggregated_profile.empty()){
        tmp.push_back({
            aggregated_profile.front().min_power + fo_profile.front().min_power,
            aggregated_profile.front().max_power + fo_profile.front().max_power
        });
        aggregated_profile.erase(aggregated_profile.begin());
        fo_profile.erase(fo_profile.begin());
    }
    while(!fo_profile.empty()){
        tmp.push_back(fo_profile.front());
        fo_profile.erase(fo_profile.begin());
    }
    while(!aggregated_profile.empty()){
        tmp.push_back(aggregated_profile.front());
        aggregated_profile.erase(aggregated_profile.begin());
    }

    double synergy  = 0.0;
    double maxPrice = 0.0;
    for(double p : spotPrices)
        if(p > maxPrice) maxPrice = p;

    for(size_t h = 0; h < tmp.size(); h++){
        double avgLoad   = (tmp[h].min_power + tmp[h].max_power)*0.5;
        int hourIndex    = (offset + h) % spotPrices.size();
        double price = spotPrices[hourIndex];
        synergy += avgLoad * (maxPrice - price);
    }

    if(synergy > best_synergy){
        best_synergy = synergy;
    }
    return tmp;
}
