#include <cmath>
#include <iostream>
#include <algorithm>

#include "../include/alignments.h"

using namespace std;

Flexoffer least_flexible_object(vector<Flexoffer>&);
Tec_flexoffer least_flexible_object(vector<Tec_flexoffer> &offers);
vector<TimeSlice> calc_alignment(vector<TimeSlice>, Flexoffer, int, double&); 
vector<TimeSlice> calc_alignment(vector<TimeSlice>, Tec_flexoffer, int, double&);
//vector<TimeSlice> calc_priceAwareAlignment(vector<TimeSlice> &aggregated_profile, const Flexoffer &least_flexible,int offset,double &best_synergy, const vector<double> &spotPrices);
//vector<TimeSlice> calc_priceAwareAlignment(vector<TimeSlice> &aggregated_profile,const Tec_flexoffer &least_flexible,int offset,double &best_synergy, const vector<double> &spotPrices);

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
        aggregated_end_time = min(aggregated_end_time, fo.get_et());
        aggregated_latest = aggregated_end_time - (duration * 3600);
        overall_min += fo.get_min_overall_kw();
        overall_max += fo.get_max_overall_kw();
        aggregated_profile = tmp;
        tmp.clear();
    }
}

//for fo
void balance_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, vector<TimeSlice> &aggregated_profile, int &duration, vector<Flexoffer> offers){

    Flexoffer least_flexible = least_flexible_object(offers);  
    cout << "least flexi: " << least_flexible.get_profile().size() << endl;
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
                aggregated_end_time = least_flexible.get_et(); 
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
                aggregated_end_time = least_flexible.get_et(); 
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
            double diff_sec = difftime(least_flexible.get_est(), aggregated_latest);
            int empty_space = (int) ceil(diff_sec / 3600.0);
            for(int i = 0; i < empty_space; i++) aggregated_profile.push_back({0,0});
            for(auto slice : least_flexible.get_profile()) aggregated_profile.push_back(slice);
            aggregated_latest = least_flexible.get_lst(); 
            aggregated_end_time += (least_flexible.get_duration()*3600); 
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
                    best_earliest = aggregated_earliest + ((i-1) * 3600);
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


static const int DAY_HOURS = 24;

void priceAwareAlignment(time_t &agg_est,time_t &agg_lst,time_t &agg_end,std::vector<TimeSlice> &agg_profile,int &duration,std::vector<Flexoffer> offers,const std::vector<double> &spotPrices){
    
    // 1) Lambda to compute synergy of a candidate profile. (simlar to the calc_alignment for balanced )
    auto synergy = [&](const vector<TimeSlice> &prof) {
        if (prof.empty() || spotPrices.empty()) return 0.0;
        double maxP = *max_element(spotPrices.begin(), spotPrices.end());
        double val  = 0.0;
        int T = min<int>(prof.size(), DAY_HOURS);
        for (int i=0; i < T; i++) {
            double load = 0.5 * (prof[i].min_power + prof[i].max_power);
            double locP = spotPrices[i];
            val += load * (maxP - locP);
        }
        return val;
    };

    // 2) Lambda to merge an FO at integer 'offset' hours. 
    //    We clamp offsets so we never exceed 0..24 in size.
    auto mergeAtOffset = [&](const std::vector<TimeSlice> &base, const Flexoffer &fo, int offset) {
        // Make a copy of the base aggregator
        std::vector<TimeSlice> result = base;

        auto foProf = fo.get_profile();
        int foDur   = (int)foProf.size();

        // (a) If offset < 0, clamp to 0 so we do not shift aggregator "behind hour 0".
        if (offset < 0) offset = 0;

        // (b) If offset + foDur > 24, also clamp so we don't exceed day length
        if (offset + foDur > DAY_HOURS) {
            // We skip partial merges or reduce foDur to fit
            foDur = std::max(0, DAY_HOURS - offset);
        }

        // (c) Ensure 'result' has at least DAY_HOURS slices. 
        //     We won't use more than 24 anyway, but we must store partial merges.
        if ((int)result.size() < DAY_HOURS) {
            result.resize(DAY_HOURS, {0.0, 0.0});
        }

        // (d) Merge FO slices in [offset..offset+foDur-1]
        for (int i = 0; i < foDur; i++) {
            result[offset + i].min_power += foProf[i].min_power;
            result[offset + i].max_power += foProf[i].max_power;
        }
        // Finally, keep only the first 24 slices.
        result.resize(DAY_HOURS);
        return result;
    };

    // 3) Initialize aggregator with the *last* FO in the vector
    //    (the original code does pop_back).
    Flexoffer start = offers.back();
    offers.pop_back();

    agg_est = start.get_est();
    agg_lst = start.get_lst();
    agg_end = start.get_et();
    agg_profile = start.get_profile();

    // Trim aggregator to max 24 hours if needed
    if ((int)agg_profile.size() > DAY_HOURS) {
        agg_profile.resize(DAY_HOURS);
    }
    duration = (int)agg_profile.size();

    // Helper to recalc aggregator times (still naive: earliest + duration, etc.)
    auto updateAggregator = [&](time_t newEst, const std::vector<TimeSlice> &prof) {
        agg_est    = newEst;
        duration   = std::min<int>((int)prof.size(), DAY_HOURS);
        agg_end    = agg_est + duration * 3600;
        agg_lst    = agg_end - duration * 3600;
    };

    // Initialize aggregator from the start FO
    updateAggregator(agg_est, agg_profile);

    // 4) For each remaining FO, pick the best offset in [0..24-foDur]
    //    that yields max synergy. We skip negative offsets or expansions beyond 24.
    while (!offers.empty()) {
        Flexoffer curr = offers.back();
        offers.pop_back();

        // earliest-latest difference in seconds => in hours
        double offMinSec = difftime(curr.get_est(), agg_est); 
        double offMaxSec = difftime(curr.get_lst(), agg_est);

        // floor to 0, clamp if negative
        int offMin = std::max(0, (int)std::floor(offMinSec/3600.0));
        // clamp at 24 so we don't exceed a single day
        int offMax = std::min(DAY_HOURS, (int)std::ceil(offMaxSec/3600.0));

        double bestVal = -1e15;
        std::vector<TimeSlice> bestProf = agg_profile;
        time_t bestEst = agg_est;

        for (int off = offMin; off <= offMax; off++) {
            // Try merging at offset 'off'
            auto cand = mergeAtOffset(agg_profile, curr, off);
            double s  = synergy(cand);
            if (s > bestVal) {
                bestVal  = s;
                bestProf = cand;
                // aggregator earliest might remain the same
                // (we skip negative offsets, so no earlier than current agg_est)
                bestEst  = agg_est;
            }
        }

        // commit the best found
        agg_profile = bestProf;
        updateAggregator(bestEst, agg_profile);
    }
}

void priceAwareAlignment(time_t &agg_est,time_t &agg_lst,time_t &agg_end,std::vector<TimeSlice> &agg_profile,int &duration,double &overall_min,double &overall_max,std::vector<Tec_flexoffer> offers,const std::vector<double> &spotPrices){

    // 1) This is the same as for Normal FOs.
    auto computeSynergy = [&](const std::vector<TimeSlice> &prof){
        if (prof.empty() || spotPrices.empty()) return 0.0;
        double maxP = *max_element(spotPrices.begin(), spotPrices.end());
        double val = 0.0;
        int T = min<int>(prof.size(), DAY_HOURS);
        for (int h = 0; h < T; h++) {
            double load = 0.5 * (prof[h].min_power + prof[h].max_power);
            double locP = spotPrices[h];
            val += load * (maxP - locP);
        }
        return val;
    };

    auto mergeTecAtOffset = [&](const vector<TimeSlice> &base,const Tec_flexoffer &fo, int offsetHour){
        std::vector<TimeSlice> result = base;
        auto foProf = fo.get_profile();
        int  foDur  = (int)foProf.size();
        if (offsetHour < 0) offsetHour = 0;
        if (offsetHour + foDur > DAY_HOURS) {
            foDur = max(0, DAY_HOURS - offsetHour);
        }

        if ((int)result.size() < DAY_HOURS) {
            result.resize(DAY_HOURS, {0.0, 0.0});
        }

        for (int i = 0; i < foDur; i++) {
            result[offsetHour + i].min_power += foProf[i].min_power;
            result[offsetHour + i].max_power += foProf[i].max_power;
        }
        result.resize(DAY_HOURS);
        return result;
    };

    Tec_flexoffer start = offers.back();
    offers.pop_back();

    agg_est = start.get_est();
    agg_lst = start.get_lst();
    agg_end = start.get_et();

    agg_profile = start.get_profile();
    if ((int)agg_profile.size() > DAY_HOURS) {
        agg_profile.resize(DAY_HOURS);
    }
    duration = (int)agg_profile.size();

    // aggregator overall min & max
    overall_min = start.get_min_overall_kw();
    overall_max = start.get_max_overall_kw();

    // Helper to recalc aggregator earliest/end times + overall min & max
    auto updateAggregator = [&](time_t newEst, const std::vector<TimeSlice> &prof, double minVal, double maxVal){
        agg_est = newEst;
        duration = std::min<int>((int)prof.size(), DAY_HOURS);
        agg_end = agg_est + duration * 3600;
        agg_lst = agg_end - (duration * 3600);

        overall_min = minVal;
        overall_max = maxVal;
    };

    // small lambda to recalc aggregator’s overall min & max from a timeslice array
    auto recalcOverall = [&](const std::vector<TimeSlice> &prof){
        double mSum=0.0, MSum=0.0;
        int T = std::min<int>(prof.size(), DAY_HOURS);
        for (int i=0; i < T; i++){
            mSum += prof[i].min_power;
            MSum += prof[i].max_power;
        }
        return std::pair<double,double>(mSum, MSum);
    };

    // init aggregator from 'start' FO
    updateAggregator(agg_est, agg_profile, overall_min, overall_max);

    // 4) For each remaining TEC FO, choose best offset in [0..24-foDur]
    while (!offers.empty()) {
        Tec_flexoffer curr = offers.back();
        offers.pop_back();

        double offMinSec = difftime(curr.get_est(), agg_est);
        double offMaxSec = difftime(curr.get_lst(), agg_est);

        int offMin = std::max(0, (int)std::floor(offMinSec / 3600.0));
        int offMax = std::min(DAY_HOURS, (int)std::ceil(offMaxSec / 3600.0));

        double bestVal = -1e15;
        std::vector<TimeSlice> bestProf = agg_profile;
        std::pair<double,double> bestLimits = {overall_min, overall_max};
        time_t bestEst = agg_est;

        for (int off = offMin; off <= offMax; off++) {
            // attempt merging
            auto cand = mergeTecAtOffset(agg_profile, curr, off);
            auto synergyVal = computeSynergy(cand);

            // recalc aggregator’s overall min & max 
            auto [mSum, MSum] = recalcOverall(cand);

            // if synergy is better, commit
            if (synergyVal > bestVal) {
                bestVal    = synergyVal;
                bestProf   = cand;
                bestLimits = {mSum, MSum};
                bestEst    = agg_est; // we skip negative offsets
            }
        }

        // commit
        agg_profile = bestProf;
        updateAggregator(bestEst, bestProf, bestLimits.first, bestLimits.second);
    }
}
