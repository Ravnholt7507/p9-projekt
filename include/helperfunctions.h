#ifndef HELPERFUNCTION_H
#define HELPERFUNCTION_H

#include <vector>
#include <string>
#include <variant>
#include "flexoffer.h"
#include "parsing.h"
#include "group.h"
#include "DFO.h"
#include "tec.h"

using namespace std;

struct MBR {
    int min_est_hour;
    int max_est_hour;
    int min_lst_hour;
    int max_lst_hour;
};

void createMBR(const Fo_Group& group, MBR& mbr);
void createMBR(const Tec_Group& group, MBR& mbr);
bool exceedsThreshold(const MBR& mbr, int est_threshold, int lst_threshold);
bool exceedThresholds(int minE, int maxE, int minL, int maxL,int est_threshold, int lst_threshold);
void clusterGroup(std::vector<Fo_Group>& groups, int est_threshold, int lst_threshold, int max_group_size);
void clusterGroup(std::vector<Tec_Group>& groups, int est_threshold, int lst_threshold, int max_group_size);
vector<AggregatedFlexOffer> nToMAggregation(vector<Flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, int startGroupId);
vector<AggregatedFlexOffer> nToMAggregation(vector<Flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, const vector<double> &spotPrices, int startGroupId);
vector<AggregatedFlexOffer> nToMAggregation(vector<Tec_flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, int startGroupId);
vector<AggregatedFlexOffer> nToMAggregation(vector<Tec_flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, const vector<double> &spotPrices, int startGroupId);

#endif // HELPERFUNCTION_H
