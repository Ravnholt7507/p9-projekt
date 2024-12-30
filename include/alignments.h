#ifndef ALIGNMENTS_H
#define ALIGNMENTS_H
#include <ctime>
#include "../include/flexoffer.h"
enum class Alignments {start, balance};
void start_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time,
                     vector<TimeSlice> &aggregated_profile, int &duration, const vector<Flexoffer> &offers);
void balance_alignment(time_t &aggregated_earliest, time_t &aggregated_latest, time_t &aggregated_end_time,
                     vector<TimeSlice> &aggregated_profile, int &duration, vector<Flexoffer> offers);
#endif
