#ifndef PARSING_H
#define PARSING_H

#include <vector>
#include <string>
#include <variant>
#include "flexoffer.h"
#include "group.h"
#include "DFO.h"
#include "tec.h"

using namespace std;

time_t roundToNearestHour(time_t raw_time);
std::vector<double> readSpotPricesFromCSV(const string& filename);
vector<variant<Flexoffer, Tec_flexoffer>> parseEVDataToFlexOffers(const string& filename, int type);
void dumpMetricsToCSV(const std::string& filename, const std::vector<std::string>& headers, const std::vector<std::vector<double>>& data);
void prepareAndDumpMetrics(const std::vector<double>& spotPrices, const std::vector<AggregatedFlexOffer>& afos, const std::string& csvFilePath, const std::string& pythonScriptPath);
tuple<vector<double>, vector<double>, vector<double>, vector<double>> getFRCprices(const string& filename);
void dumpFCRDataToCSV(const vector<vector<double>> &powerVars,const vector<vector<double>> &upVars,const vector<vector<double>> &downVars,double totalRevenue,const string &filename);
void dumpSolverAndDisaggResults(vector<AggregatedFlexOffer> &afos, vector<double> &spotPrices, const string &aggCsvPath, const string &disCsvPath);
vector<DFO> parseEVDataToDFO(const string &filename, int numsamples);

#endif // PARSING_H
