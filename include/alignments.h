#ifndef ALIGNMENTS_H
#define ALIGNMENTS_H
#include <ctime>
#include "flexoffer.h"
#include "tec.h"
enum class Alignments {start, balance, price};
void start_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, vector<TimeSlice> &aggregated_profile, int &duration, const vector<Flexoffer> &offers);
void start_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time,vector<TimeSlice> &aggregated_profile, int &duration, double &overall_min, double &overall_max, const vector<Tec_flexoffer> &offers);
void balance_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, vector<TimeSlice> &aggregated_profile, int &duration, vector<Flexoffer> offers);
void balance_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time, vector<TimeSlice> &aggregated_profile, int &duration, double &overall_min, double &overall_max, vector<Tec_flexoffer> offers);
void priceAwareAlignment(time_t &aggregated_earliest,time_t &aggregated_latest,time_t &aggregated_end_time,std::vector<TimeSlice> &aggregated_profile,int &duration,std::vector<Flexoffer> offers,const std::vector<double> &spotPrices);
void priceAwareAlignment(time_t &aggregated_earliest,time_t &aggregated_latest,time_t &aggregated_end_time,vector<TimeSlice> &aggregated_profile,int &duration,double &overall_min,double &overall_max,vector<Tec_flexoffer> offers, const vector<double> &spotPrices);
#endif
