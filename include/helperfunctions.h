#ifndef HELPERFUNCTION_H
#define HELPERFUNCTION_H

#include <vector>
#include <string>
#include <variant>
#include "flexoffer.h"
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
bool exceedThresholds(
    int minE, int maxE, int minL, int maxL,
    int est_threshold, int lst_threshold
);
void clusterGroup(std::vector<Fo_Group>& groups, int est_threshold, int lst_threshold, int max_group_size);
void clusterGroup(std::vector<Tec_Group>& groups, int est_threshold, int lst_threshold, int max_group_size);
time_t roundToNearestHour(time_t raw_time);
std::vector<double> readSpotPricesFromCSV(const string& filename);
vector<variant<Flexoffer, Tec_flexoffer>> parseEVDataToFlexOffers(const string& filename, int type);
void dumpMetricsToCSV(const std::string& filename, const std::vector<std::string>& headers, const std::vector<std::vector<double>>& data);
void prepareAndDumpMetrics(const std::vector<double>& spotPrices, const std::vector<AggregatedFlexOffer>& afos, const std::string& csvFilePath, const std::string& pythonScriptPath);
tuple<vector<double>, vector<double>, vector<double>, vector<double>> getFRCprices(const string& filename);
void dumpFCRDataToCSV(const vector<vector<double>> &powerVars,const vector<vector<double>> &upVars,const vector<vector<double>> &downVars,double totalRevenue,const string &filename);
void dumpSolverAndDisaggResults(vector<AggregatedFlexOffer> &afos, vector<double> &spotPrices, const string &aggCsvPath, const string &disCsvPath);
vector<AggregatedFlexOffer> nToMAggregation(vector<Flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, int startGroupId);
vector<AggregatedFlexOffer> nToMAggregation(vector<Flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, const vector<double> &spotPrices, int startGroupId);
vector<AggregatedFlexOffer> nToMAggregation(vector<Tec_flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, int startGroupId);
vector<AggregatedFlexOffer> nToMAggregation(vector<Tec_flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align, const vector<double> &spotPrices, int startGroupId);
vector<DFO> parseEVDataToDFO(const string &filename, int numsamples);

#endif // HELPERFUNCTION_H
