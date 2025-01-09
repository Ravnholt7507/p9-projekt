#include <cmath>
#include <algorithm>

#include "../include/alignments.h"

using namespace std;

Flexoffer least_flexible_object(vector<Flexoffer>&);
Tec_flexoffer least_flexible_object(vector<Tec_flexoffer> &offers);
vector<TimeSlice> calc_alignment(vector<TimeSlice>, Flexoffer, int, double&); 
vector<TimeSlice> calc_alignment(vector<TimeSlice>, Tec_flexoffer, int, double&);
// vector<TimeSlice> calc_priceAwareAlignment(vector<TimeSlice> &aggregated_profile, const Flexoffer &least_flexible,int offset,double &best_synergy, const vector<double> &spotPrices);
// vector<TimeSlice> calc_priceAwareAlignment(vector<TimeSlice> &aggregated_profile,const Tec_flexoffer &least_flexible,int offset,double &best_synergy, const vector<double> &spotPrices);

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



void priceAwareAlignment(
    time_t &aggregated_earliest,
    time_t &aggregated_latest,
    time_t &aggregated_end_time,
    std::vector<TimeSlice> &aggregated_profile,
    int &duration,
    std::vector<Flexoffer> offers,
    const std::vector<double> &spotPrices)
{
    if (offers.empty()) {
        aggregated_profile.clear();
        duration       = 0;
        aggregated_earliest = 0;
        aggregated_latest   = 0;
        aggregated_end_time = 0;
        return;
    }

    auto computeSynergy = [&](const std::vector<TimeSlice> &candidateProfile) -> double
    {
        if (candidateProfile.empty() || spotPrices.empty()) return 0.0;
        double maxPrice = *std::max_element(spotPrices.begin(), spotPrices.end());
        double synergy  = 0.0;
        int T = static_cast<int>(candidateProfile.size());

        for (int h = 0; h < T; h++)
        {
            double avgLoad = 0.5 * (candidateProfile[h].min_power + candidateProfile[h].max_power);
            double localPrice = spotPrices[h % spotPrices.size()];
            synergy += avgLoad * (maxPrice - localPrice);
        }
        return synergy;
    };

    auto mergeTecAtOffset = [&](const std::vector<TimeSlice> &aggregatorProfile,
                                const Flexoffer &offer,
                                int offsetHour) -> std::vector<TimeSlice>
    {
        std::vector<TimeSlice> result = aggregatorProfile;
        auto foProfile = offer.get_profile();
        int foDur = static_cast<int>(foProfile.size());

        while (offsetHour < 0)
        {
            result.insert(result.begin(), {0.0, 0.0});
            offsetHour++;
        }

        while (static_cast<int>(result.size()) < offsetHour + foDur)
        {
            result.push_back({0.0, 0.0});
        }

        for (int i = 0; i < foDur; i++)
        {
            result[offsetHour + i].min_power += foProfile[i].min_power;
            result[offsetHour + i].max_power += foProfile[i].max_power;
        }
        return result;
    };

    Flexoffer starter = offers.back();
    offers.pop_back();

    aggregated_earliest = starter.get_est();
    aggregated_latest   = starter.get_lst();
    aggregated_end_time = starter.get_et();
    aggregated_profile  = starter.get_profile();
    duration            = static_cast<int>(aggregated_profile.size());

    auto updateAggregator = [&](time_t newStart, const std::vector<TimeSlice> &prof) {
        aggregated_earliest = newStart;
        duration            = static_cast<int>(prof.size());
        aggregated_end_time = aggregated_earliest + duration * 3600;
        aggregated_latest   = aggregated_end_time - duration * 3600;
    };

    updateAggregator(aggregated_earliest, aggregated_profile);

    while (!offers.empty())
    {
        Flexoffer current = offers.back();
        offers.pop_back();

        double offsetMinSec = difftime(current.get_est(), aggregated_earliest);
        double offsetMaxSec = difftime(current.get_lst(), aggregated_earliest);
        int offsetMin = static_cast<int>(std::floor(offsetMinSec / 3600.0));
        int offsetMax = static_cast<int>(std::ceil (offsetMaxSec / 3600.0));

        double bestSynergy = -std::numeric_limits<double>::infinity();
        std::vector<TimeSlice> bestProfile = aggregated_profile;
        time_t bestStart = aggregated_earliest;

        for (int testOffset = offsetMin; testOffset <= offsetMax; testOffset++)
        {
            auto candidate = mergeTecAtOffset(aggregated_profile, current, testOffset);

            double synergyVal = computeSynergy(candidate);
            if (synergyVal > bestSynergy)
            {
                bestSynergy = synergyVal;
                bestProfile = candidate;
                // aggregator earliest might shift if testOffset < 0
                time_t newStart = aggregated_earliest + (testOffset >= 0 ? 0 : testOffset * 3600);
                bestStart = newStart;
            }
        }
        // commit
        aggregated_profile = bestProfile;
        updateAggregator(bestStart, aggregated_profile);
    }
}


void priceAwareAlignment(
    time_t &aggregated_earliest,
    time_t &aggregated_latest,
    time_t &aggregated_end_time,
    std::vector<TimeSlice> &aggregated_profile,
    int &duration,
    double &overall_min,
    double &overall_max,
    std::vector<Tec_flexoffer> offers,
    const std::vector<double> &spotPrices)
{
    if (offers.empty()) {
        aggregated_profile.clear();
        duration       = 0;
        overall_min    = 0.0;
        overall_max    = 0.0;
        aggregated_earliest = 0;
        aggregated_latest   = 0;
        aggregated_end_time = 0;
        return;
    }

    auto computeSynergy = [&](const std::vector<TimeSlice> &candidateProfile) -> double
    {
        if (candidateProfile.empty() || spotPrices.empty()) return 0.0;
        double maxPrice = *std::max_element(spotPrices.begin(), spotPrices.end());
        double synergy  = 0.0;
        int T = static_cast<int>(candidateProfile.size());

        for (int h = 0; h < T; h++)
        {
            double avgLoad = 0.5 * (candidateProfile[h].min_power + candidateProfile[h].max_power);
            double localPrice = spotPrices[h % spotPrices.size()];
            synergy += avgLoad * (maxPrice - localPrice);
        }
        return synergy;
    };

    auto mergeTecAtOffset = [&](const std::vector<TimeSlice> &aggregatorProfile,
                                const Tec_flexoffer &offer,
                                int offsetHour) -> std::vector<TimeSlice>
    {
        std::vector<TimeSlice> result = aggregatorProfile;
        auto foProfile = offer.get_profile();
        int foDur = static_cast<int>(foProfile.size());

        while (offsetHour < 0)
        {
            result.insert(result.begin(), {0.0, 0.0});
            offsetHour++;
        }

        while (static_cast<int>(result.size()) < offsetHour + foDur)
        {
            result.push_back({0.0, 0.0});
        }

        for (int i = 0; i < foDur; i++)
        {
            result[offsetHour + i].min_power += foProfile[i].min_power;
            result[offsetHour + i].max_power += foProfile[i].max_power;
        }
        return result;
    };

    Tec_flexoffer starter = offers.back();
    offers.pop_back();

    aggregated_earliest = starter.get_est();
    aggregated_latest   = starter.get_lst();
    aggregated_end_time = starter.get_et();
    aggregated_profile  = starter.get_profile();
    duration            = static_cast<int>(aggregated_profile.size());

    auto updateAggregator = [&](time_t newStart, const std::vector<TimeSlice> &prof) {
        aggregated_earliest = newStart;
        duration            = static_cast<int>(prof.size());
        aggregated_end_time = aggregated_earliest + duration * 3600;
        aggregated_latest   = aggregated_end_time - duration * 3600;

        double totMin=0.0, totMax=0.0;
        for (auto &ts : prof) {
            totMin += ts.min_power;
            totMax += ts.max_power;
        }
        overall_min = totMin;
        overall_max = totMax;
    };

    updateAggregator(aggregated_earliest, aggregated_profile);

    while (!offers.empty())
    {
        Tec_flexoffer current = offers.back();
        offers.pop_back();

        double offsetMinSec = difftime(current.get_est(), aggregated_earliest);
        double offsetMaxSec = difftime(current.get_lst(), aggregated_earliest);
        int offsetMin = static_cast<int>(std::floor(offsetMinSec / 3600.0));
        int offsetMax = static_cast<int>(std::ceil (offsetMaxSec / 3600.0));

        double bestSynergy = -std::numeric_limits<double>::infinity();
        std::vector<TimeSlice> bestProfile = aggregated_profile;
        time_t bestStart = aggregated_earliest;

        for (int testOffset = offsetMin; testOffset <= offsetMax; testOffset++)
        {
            auto candidate = mergeTecAtOffset(aggregated_profile, current, testOffset);

            double synergyVal = computeSynergy(candidate);
            if (synergyVal > bestSynergy)
            {
                bestSynergy = synergyVal;
                bestProfile = candidate;
                // aggregator earliest might shift if testOffset < 0
                time_t newStart = aggregated_earliest + (testOffset >= 0 ? 0 : testOffset * 3600);
                bestStart = newStart;
            }
        }
        // commit
        aggregated_profile = bestProfile;
        updateAggregator(bestStart, aggregated_profile);
    }
}