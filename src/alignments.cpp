#include <cmath>
#include <iostream>
#include <algorithm>

#include "../include/alignments.h"

using namespace std;

Flexoffer least_flexible_object(vector<Flexoffer>&);
Tec_flexoffer least_flexible_object(vector<Tec_flexoffer> &offers);
vector<TimeSlice> calc_alignment(vector<TimeSlice>, Flexoffer, int, double&); 
vector<TimeSlice> calc_alignment(vector<TimeSlice>, Tec_flexoffer, int, double&);
void mergeFlexAtOffset(vector<TimeSlice> &base, const Flexoffer &fo, int offset);
void expandToFitFront(time_t &aggEarliest, int &duration, vector<TimeSlice> &aggProfile, const Flexoffer &fo);
void expandToFitBack(time_t &aggEarliest, int &duration,vector<TimeSlice> &aggProfile, const Flexoffer &fo);
double synergyFunction(const vector<TimeSlice> &prof, time_t aggregatorEarliest, const vector<double> &spotPrices);
void mergeTecAtOffset(vector<TimeSlice> &base, const Tec_flexoffer &fo, int offset);
void expandTecFront(time_t &aggEarliest,int &duration,vector<TimeSlice> &aggProfile,double &overallMin,double &overallMax,const Tec_flexoffer &fo);
void expandTecBack(time_t &aggEarliest,int &duration,vector<TimeSlice> &aggProfile,double &overallMin,double &overallMax,const Tec_flexoffer &fo);
double synergyTec(const vector<TimeSlice> &prof,time_t aggregatorEarliest,const vector<double> &spotPrices);


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
        aggregated_end_time = max(aggregated_end_time, fo.get_et());
        aggregated_latest = aggregated_end_time - (duration * 3600);
        if(aggregated_latest < aggregated_earliest) aggregated_earliest = aggregated_latest;
        aggregated_profile = tmp;
        tmp.clear();
    }
}

//for tec
void start_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, vector<TimeSlice> &aggregated_profile, int &duration, double &overall_min, double &overall_max, vector<Tec_flexoffer> offers){
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
        aggregated_end_time = max(aggregated_end_time, fo.get_et());
        aggregated_latest = aggregated_end_time - (duration * 3600);
        if(aggregated_latest < aggregated_earliest) aggregated_earliest = aggregated_latest;
        overall_min += fo.get_min_overall_kw();
        overall_max += fo.get_max_overall_kw();
        aggregated_profile = tmp;
        tmp.clear();
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
            if(aggregated_latest + (duration * 3600) < least_flexible.get_est()){
                double diff_sec = difftime(least_flexible.get_est(), aggregated_latest + (duration * 3600));
                int empty_space = ceil(diff_sec / 3600);
                for(int i = 0; i < empty_space; i++) aggregated_profile.push_back({0,0});
                for(auto slice : least_flexible.get_profile()) aggregated_profile.push_back(slice);
                aggregated_end_time = least_flexible.get_est() + (least_flexible.get_duration() * 3600); 
                duration = aggregated_profile.size();
                aggregated_latest = aggregated_end_time - (duration * 3600); 
                if(aggregated_latest < aggregated_earliest) aggregated_earliest = aggregated_latest;
            } else {
                double diff_sec = difftime(least_flexible.get_est(), aggregated_latest);
                int empty_space = ceil(diff_sec / 3600);
                vector<TimeSlice> tmp = least_flexible.get_profile();
                while(!tmp.empty()){
                    if(empty_space > duration-1){
                        aggregated_profile.push_back(tmp[0]);
                    } else {
                        aggregated_profile[empty_space] = {aggregated_profile[empty_space].min_power + tmp[0].min_power,
                                                           aggregated_profile[empty_space].max_power + tmp[0].max_power};
                    }
                    tmp.erase(tmp.begin());
                    empty_space++;
                }
                aggregated_end_time = max(least_flexible.get_est() + (least_flexible.get_duration() * 3600), aggregated_end_time); 
                duration = aggregated_profile.size();
                aggregated_latest = aggregated_end_time - (duration * 3600); 
                if(aggregated_latest < aggregated_earliest) aggregated_earliest = aggregated_latest;
            }
        } else if(aggregated_earliest > least_flexible.get_lst()){
            if(least_flexible.get_lst() + (least_flexible.get_duration() * 3600) < aggregated_earliest){
                double diff_sec = difftime(aggregated_earliest, least_flexible.get_lst() + (least_flexible.get_duration()));
                int empty_space = ceil(diff_sec / 3600);
                for(int i = 0; i < empty_space; i++) aggregated_profile.insert(aggregated_profile.begin(), {0,0});
                vector<TimeSlice> tmp = least_flexible.get_profile();
                reverse(tmp.begin(), tmp.end());
                for(auto slice : tmp) aggregated_profile.insert(aggregated_profile.begin(), slice);
                aggregated_earliest = least_flexible.get_lst();
                duration = aggregated_profile.size();
            } else {
                double diff_sec = difftime(aggregated_earliest, least_flexible.get_lst());
                int empty_space = ceil(diff_sec / 3600);
                vector<TimeSlice> tmp = least_flexible.get_profile();
                for(int i = 0; i < empty_space; i++){ 
                    aggregated_profile.insert(aggregated_profile.begin(), tmp[0]);
                    tmp.erase(tmp.begin());
                }
                while(!tmp.empty()){
                    aggregated_profile[empty_space] = {aggregated_profile[empty_space].min_power + tmp[0].min_power,
                                                       aggregated_profile[empty_space].max_power + tmp[0].max_power};
                    tmp.erase(tmp.begin());
                    empty_space++;
                }
                aggregated_earliest = least_flexible.get_lst();
                duration = aggregated_profile.size();
                aggregated_latest = aggregated_end_time - (duration * 3600); 
            }
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
                    best_endtime = best_latest + (best_duration*3600);
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

            if(aggregated_latest + (duration * 3600) < least_flexible.get_est()){
                double diff_sec = difftime(least_flexible.get_est(), aggregated_latest + (duration * 3600));
                int empty_space = ceil(diff_sec / 3600);
                for(int i = 0; i < empty_space; i++) aggregated_profile.push_back({0,0});
                for(auto slice : least_flexible.get_profile()) aggregated_profile.push_back(slice);
                aggregated_end_time = least_flexible.get_est() + (least_flexible.get_duration() * 3600); 
                duration = aggregated_profile.size();
                aggregated_latest = aggregated_end_time - (duration * 3600); 
                overall_min += least_flexible.get_min_overall_kw();
                overall_max += least_flexible.get_max_overall_kw();
                if(aggregated_latest < aggregated_earliest) aggregated_earliest = aggregated_latest;
            } else {
                double diff_sec = difftime(least_flexible.get_est(), aggregated_latest);
                int empty_space = ceil(diff_sec / 3600);
                vector<TimeSlice> tmp = least_flexible.get_profile();
                while(!tmp.empty()){
                    if(empty_space > duration-1){
                        aggregated_profile.push_back(tmp[0]);
                    } else {
                        aggregated_profile[empty_space] = {aggregated_profile[empty_space].min_power + tmp[0].min_power,
                                                           aggregated_profile[empty_space].max_power + tmp[0].max_power};
                    }
                    tmp.erase(tmp.begin());
                    empty_space++;
                }
                aggregated_end_time = max(least_flexible.get_est() + (least_flexible.get_duration() * 3600), aggregated_end_time); 
                duration = aggregated_profile.size();
                aggregated_latest = aggregated_end_time - (duration * 3600); 
                overall_min += least_flexible.get_min_overall_kw();
                overall_max += least_flexible.get_max_overall_kw();
                if(aggregated_latest < aggregated_earliest) aggregated_earliest = aggregated_latest;
            }
        } else if(aggregated_earliest > least_flexible.get_lst()){
            if(least_flexible.get_lst() + (least_flexible.get_duration() * 3600) < aggregated_earliest){
                double diff_sec = difftime(aggregated_earliest, least_flexible.get_lst() + (least_flexible.get_duration()));
                int empty_space = ceil(diff_sec / 3600);
                for(int i = 0; i < empty_space; i++) aggregated_profile.insert(aggregated_profile.begin(), {0,0});
                vector<TimeSlice> tmp = least_flexible.get_profile();
                reverse(tmp.begin(), tmp.end());
                for(auto slice : tmp) aggregated_profile.insert(aggregated_profile.begin(), slice);
                aggregated_earliest = least_flexible.get_lst();
                duration = aggregated_profile.size();
                overall_min += least_flexible.get_min_overall_kw();
                overall_max += least_flexible.get_max_overall_kw();
            } else {
                double diff_sec = difftime(aggregated_earliest, least_flexible.get_lst());
                int empty_space = ceil(diff_sec / 3600);
                vector<TimeSlice> tmp = least_flexible.get_profile();
                for(int i = 0; i < empty_space; i++){ 
                    aggregated_profile.insert(aggregated_profile.begin(), tmp[0]);
                    tmp.erase(tmp.begin());
                }
                while(!tmp.empty()){
                    aggregated_profile[empty_space] = {aggregated_profile[empty_space].min_power + tmp[0].min_power,
                                                       aggregated_profile[empty_space].max_power + tmp[0].max_power};
                    tmp.erase(tmp.begin());
                    empty_space++;
                }
                aggregated_earliest = least_flexible.get_lst();
                duration = aggregated_profile.size();
                aggregated_latest = aggregated_end_time - (duration * 3600); 
                overall_min += least_flexible.get_min_overall_kw();
                overall_max += least_flexible.get_max_overall_kw();
            }
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



void mergeFlexAtOffset(vector<TimeSlice>& base, const Flexoffer &fo, int offset) {
    auto prof = fo.get_profile();
    int dur   = fo.get_duration();
    if (offset + dur > (int)base.size()) base.resize(offset + dur, {0,0});
    for (int i=0; i<dur; i++){
        base[offset + i].min_power += prof[i].min_power;
        base[offset + i].max_power += prof[i].max_power;
    }
}

void mergeTecAtOffset(vector<TimeSlice> &base, const Tec_flexoffer &fo, int offset){
    auto prof = fo.get_profile();
    int dur   = fo.get_duration();
    if (offset + dur > (int)base.size()) base.resize(offset + dur, {0,0});
    for (int i=0; i<dur; i++){
        base[offset + i].min_power += prof[i].min_power;
        base[offset + i].max_power += prof[i].max_power;
    }
}

double synergyFunction(const vector<TimeSlice> &prof, time_t aggEarliest, const vector<double> &prices) {
    if (prof.empty()) return 0;
    double mx = *max_element(prices.begin(), prices.end());
    tm *tmInfo = localtime(&aggEarliest);
    int off = tmInfo->tm_hour; 
    int T = (int)prof.size();
    double val=0;
    for (int i=0; i<T; i++){
        double load = 0.5*(prof[i].min_power + prof[i].max_power);
        double locP = prices[i+off];
        val += load*(mx-locP);
    }
    return val;
}

double synergyTec(const vector<TimeSlice> &prof, time_t aggEarliest, const vector<double> &prices){
    if (prof.empty()) return 0;
    double mx = *max_element(prices.begin(), prices.end());
    tm *tmInfo = localtime(&aggEarliest);
    int off = tmInfo->tm_hour;
    int T = (int)prof.size();
    double val=0;
    for(int i=0; i<T; i++){
        double load=0.5*(prof[i].min_power + prof[i].max_power);
        double locP= prices[i+off];
        val += load*(mx-locP);
    }
    return val;
}

void expandToFitFront(time_t &aggEarliest, int &duration, vector<TimeSlice> &aggProf, const Flexoffer &fo){
    time_t aggregatorEnd = aggEarliest + duration*3600;
    time_t foAbsEnd = fo.get_lst() + fo.get_duration()*3600;
    if (difftime(aggEarliest, foAbsEnd)>0){
        int needed = (int)ceil(difftime(aggEarliest, foAbsEnd)/3600.0);
        for(int i=0;i<needed;i++) aggProf.insert(aggProf.begin(), {0,0});
        aggEarliest -= needed*3600;
        duration    += needed;
        int off = (int)floor( (fo.get_est()-aggEarliest)/3600.0 );
        if(off<0) off=0;
        mergeFlexAtOffset(aggProf, fo, off);
    }
}

void expandToFitBack(time_t &aggEarliest,int &duration,vector<TimeSlice> &aggProf, const Flexoffer &fo){
    time_t aggregatorEnd = aggEarliest + duration*3600;
    time_t foAbsStart    = fo.get_est();
    if(difftime(foAbsStart, aggregatorEnd)>0){
        int needed = (int)ceil(difftime(foAbsStart, aggregatorEnd)/3600.0);
        for(int i=0;i<needed;i++) aggProf.push_back({0,0});
        duration += needed;
        int off= (int)aggProf.size()-fo.get_duration();
        if(off<0) off=0;
        mergeFlexAtOffset(aggProf, fo, off);
    }
}

void expandTecFront(time_t &aggEarliest,int &duration,vector<TimeSlice> &aggProf,double &overMin,double &overMax,const Tec_flexoffer &fo){
    time_t aggregatorEnd= aggEarliest+duration*3600;
    time_t foAbsEnd= fo.get_lst()+fo.get_duration()*3600;
    if(difftime(aggEarliest,foAbsEnd)>0){
        int needed = (int)ceil(difftime(aggEarliest, foAbsEnd)/3600.0);
        for(int i=0;i<needed;i++) aggProf.insert(aggProf.begin(),{0,0});
        aggEarliest-= needed*3600;
        duration   += needed;
        overMin+= fo.get_min_overall_kw();
        overMax+= fo.get_max_overall_kw();
        int off= (int)floor((fo.get_est()-aggEarliest)/3600.0);
        if(off<0) off=0;
        mergeTecAtOffset(aggProf,fo,off);
    }
}

void expandTecBack(time_t &aggEarliest,int &duration,vector<TimeSlice> &aggProf,double &overMin,double &overMax,const Tec_flexoffer &fo){
    time_t aggregatorEnd= aggEarliest+duration*3600;
    time_t foAbsStart= fo.get_est();
    if(difftime(foAbsStart, aggregatorEnd)>0){
        int needed= (int)ceil(difftime(foAbsStart, aggregatorEnd)/3600.0);
        for(int i=0;i<needed;i++) aggProf.push_back({0,0});
        duration+= needed;
        overMin+= fo.get_min_overall_kw();
        overMax+= fo.get_max_overall_kw();
        int off= (int)aggProf.size()-fo.get_duration();
        if(off<0) off=0;
        mergeTecAtOffset(aggProf, fo, off);
    }
}

void priceAwareAlignment(
    time_t &aggEarliest,
    time_t &aggLatest,
    time_t &aggEnd,
    vector<TimeSlice> &aggProfile,
    int &duration,
    vector<Flexoffer> offers,
    const vector<double> &spotPrices)
{
    if(offers.empty()){
        aggEarliest=aggLatest=aggEnd=0; duration=0; aggProfile.clear(); return;
    }
    Flexoffer start= least_flexible_object(offers);
    aggEarliest= start.get_est(); 
    aggLatest  = start.get_lst();
    aggEnd     = start.get_et();
    aggProfile = start.get_profile();
    duration   = aggProfile.size();

    auto updateAgg=[&](time_t newEst,const vector<TimeSlice> &prof){
        aggEarliest= newEst;
        duration= prof.size();
        aggEnd   = aggEarliest+duration*3600;
        aggLatest= aggEnd-duration*3600;
    };
    updateAgg(aggEarliest, aggProfile);

    while(!offers.empty()){
        Flexoffer fo= least_flexible_object(offers);
        time_t aggregatorEnd= aggEarliest+duration*3600;
        time_t foAbsEnd= fo.get_lst()+fo.get_duration()*3600;

        if(foAbsEnd<aggEarliest){
            expandToFitFront(aggEarliest, duration, aggProfile, fo);
            updateAgg(aggEarliest, aggProfile);
        }
        else if(fo.get_est()>aggregatorEnd){
            expandToFitBack(aggEarliest, duration, aggProfile, fo);
            updateAgg(aggEarliest, aggProfile);
        }
        else {
            double offMinSec = difftime(fo.get_est(), aggEarliest);
            double offMaxSec = difftime(fo.get_lst(), aggEarliest);
            int offMin= max(0,(int)floor(offMinSec/3600.0));
            int offMax= max(offMin,(int)ceil(offMaxSec/3600.0));
            double bestVal=0; 
            auto bestProf= aggProfile;
            for(int off=offMin; off<=offMax; off++){
                auto cand= aggProfile; 
                mergeFlexAtOffset(cand, fo, off);
                double val= synergyFunction(cand, aggEarliest, spotPrices);
                if(val>bestVal){
                    bestVal= val;
                    bestProf= cand;
                }
            }
            aggProfile= bestProf;
            updateAgg(aggEarliest, aggProfile);
        }
    }
}

void priceAwareAlignment(
    time_t &aggEarliest,
    time_t &aggLatest,
    time_t &aggEnd,
    vector<TimeSlice> &aggProfile,
    int &duration,
    double &overallMin,
    double &overallMax,
    vector<Tec_flexoffer> offers,
    const vector<double> &spotPrices)
{
    if(offers.empty()){
        aggEarliest=aggLatest=aggEnd=0; duration=0; overallMin=0; overallMax=0; aggProfile.clear(); return;
    }
    Tec_flexoffer start= least_flexible_object(offers);
    aggEarliest= start.get_est();
    aggLatest  = start.get_lst();
    aggEnd     = start.get_et();
    aggProfile = start.get_profile();
    duration   = aggProfile.size();
    overallMin = start.get_min_overall_kw();
    overallMax = start.get_max_overall_kw();

    auto updateAgg=[&](time_t newEst,const vector<TimeSlice>&prof,double mn,double mx){
        aggEarliest= newEst;
        duration   = prof.size();
        aggEnd     = aggEarliest+ duration*3600;
        aggLatest  = aggEnd - duration*3600;
        overallMin = mn;
        overallMax = mx;
    };
    updateAgg(aggEarliest, aggProfile, overallMin, overallMax);

    while(!offers.empty()){
        Tec_flexoffer fo= least_flexible_object(offers);
        time_t aggregatorEnd= aggEarliest+(duration*3600);
        time_t foAbsEnd= fo.get_lst() + fo.get_duration()*3600;

        if(foAbsEnd<aggEarliest){
            expandTecFront(aggEarliest,duration,aggProfile,overallMin,overallMax,fo);
            updateAgg(aggEarliest,aggProfile,overallMin,overallMax);
        }
        else if(fo.get_est()>aggregatorEnd){
            expandTecBack(aggEarliest,duration,aggProfile,overallMin,overallMax,fo);
            updateAgg(aggEarliest,aggProfile,overallMin,overallMax);
        }
        else {
            double offMinSec= difftime(fo.get_est(), aggEarliest);
            double offMaxSec= difftime(fo.get_lst(), aggEarliest);
            int offMin= max(0,(int)floor(offMinSec/3600.0));
            int offMax= max(offMin,(int)ceil(offMaxSec/3600.0));
            double bestVal= -1e9;
            auto   bestProf= aggProfile;
            double bestMin= overallMin;
            double bestMax= overallMax;

            for(int off= offMin; off<=offMax; off++){
                auto cand= aggProfile;
                mergeTecAtOffset(cand,fo,off);
                double val= synergyTec(cand, aggEarliest, spotPrices);
                double newMin= overallMin+ fo.get_min_overall_kw();
                double newMax= overallMax+ fo.get_max_overall_kw();
                if(val>bestVal){
                    bestVal= val;
                    bestProf= cand;
                    bestMin= newMin;
                    bestMax= newMax;
                }
            }
            aggProfile= bestProf;
            overallMin= bestMin;
            overallMax= bestMax;
            updateAgg(aggEarliest, bestProf, bestMin,bestMax);
        }
    }
}