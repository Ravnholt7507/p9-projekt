#ifndef HELPERFUNCTION_H
#define HELPERFUNCTION_H

#include <vector>
#include <string>
#include <variant>
#include "flexoffer.h"
#include "group.h"
#include "tec.h"

using namespace std;

struct MBR {
    int min_est_hour;
    int max_est_hour;
    int min_lst_hour;
    int max_lst_hour;
};

void createMBR(const Group& group, MBR& mbr);
bool exceedsThreshold(const MBR& mbr, int est_threshold, int lst_threshold);
void clusterGroup(std::vector<Group>& groups, int est_threshold, int lst_threshold, int max_group_size);
std::vector<double> readSpotPricesFromCSV(const string& filename);
vector<variant<Flexoffer, Tec_flexoffer>> parseEVDataToFlexOffers(const string& filename, int type);
void dumpMetricsToCSV(const std::string& filename, const std::vector<std::string>& headers, const std::vector<std::vector<double>>& data);
void prepareAndDumpMetrics(const std::vector<double>& spotPrices, const std::vector<AggregatedFlexOffer>& afos, const std::string& csvFilePath, const std::string& pythonScriptPath);
tuple<vector<double>, vector<double>, vector<double>, vector<double>> getFRCprices(const string& filename);
void dumpFCRDataToCSV(const vector<vector<double>> &powerVars,const vector<vector<double>> &upVars,const vector<vector<double>> &downVars,double totalRevenue,const string &filename);

#endif // HELPERFUNCTION_H
