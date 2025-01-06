#include "../include/helperfunctions.h"
#include "../include/solver.h"
#include <algorithm>
#include <fstream>
#include <chrono>
#include <fstream>
#include <chrono>  
#include <iostream>
#include <sstream>
#include <cmath>
#include <limits>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <utility>

using namespace std;

//For Fo
void createMBR(const Fo_Group& group, MBR& mbr) {
    const auto& flexoffers = group.getFlexOffers();
    if (flexoffers.empty()) return;

    mbr.min_est_hour = flexoffers.front().get_est_hour();
    mbr.max_est_hour = flexoffers.front().get_est_hour();
    mbr.min_lst_hour = flexoffers.front().get_lst_hour();
    mbr.max_lst_hour = flexoffers.front().get_lst_hour();

    for (const auto& fo : flexoffers) {
        int est_hour = fo.get_est_hour();
        int lst_hour = fo.get_lst_hour();

        if (est_hour < mbr.min_est_hour) mbr.min_est_hour = est_hour;
        if (est_hour > mbr.max_est_hour) mbr.max_est_hour = est_hour;
        if (lst_hour < mbr.min_lst_hour) mbr.min_lst_hour = lst_hour;
        if (lst_hour > mbr.max_lst_hour) mbr.max_lst_hour = lst_hour;
    }
}

//For Tec
void createMBR(const Tec_Group& group, MBR& mbr) {
    const auto& flexoffers = group.getFlexOffers();
    if (flexoffers.empty()) return;

    mbr.min_est_hour = flexoffers.front().get_est_hour();
    mbr.max_est_hour = flexoffers.front().get_est_hour();
    mbr.min_lst_hour = flexoffers.front().get_lst_hour();
    mbr.max_lst_hour = flexoffers.front().get_lst_hour();

    for (const auto& fo : flexoffers) {
        int est_hour = fo.get_est_hour();
        int lst_hour = fo.get_lst_hour();

        if (est_hour < mbr.min_est_hour) mbr.min_est_hour = est_hour;
        if (est_hour > mbr.max_est_hour) mbr.max_est_hour = est_hour;
        if (lst_hour < mbr.min_lst_hour) mbr.min_lst_hour = lst_hour;
        if (lst_hour > mbr.max_lst_hour) mbr.max_lst_hour = lst_hour;
    }
}


bool exceedsThreshold(const MBR& mbr, int est_threshold, int lst_threshold) {
    int est_range = mbr.max_est_hour - mbr.min_est_hour;
    int lst_range = mbr.max_lst_hour - mbr.min_lst_hour;
    return est_range > est_threshold || lst_range > lst_threshold;
}

//For Fo
static double groupDistance(const Fo_Group& g1, const Fo_Group& g2) {
    MBR m1, m2;
    createMBR(g1, m1);
    createMBR(g2, m2);

    double c1_est = (m1.min_est_hour + m1.max_est_hour) / 2.0;
    double c1_lst = (m1.min_lst_hour + m1.max_lst_hour) / 2.0;
    double c2_est = (m2.min_est_hour + m2.max_est_hour) / 2.0;
    double c2_lst = (m2.min_lst_hour + m2.max_lst_hour) / 2.0;

    double dx = c2_est - c1_est;
    double dy = c2_lst - c1_lst;
    return sqrt(dx*dx + dy*dy);
}

//for Tec
static double groupDistance(const Tec_Group& g1, const Tec_Group& g2) {
    MBR m1, m2;
    createMBR(g1, m1);
    createMBR(g2, m2);

    double c1_est = (m1.min_est_hour + m1.max_est_hour) / 2.0;
    double c1_lst = (m1.min_lst_hour + m1.max_lst_hour) / 2.0;
    double c2_est = (m2.min_est_hour + m2.max_est_hour) / 2.0;
    double c2_lst = (m2.min_lst_hour + m2.max_lst_hour) / 2.0;

    double dx = c2_est - c1_est;
    double dy = c2_lst - c1_lst;
    return sqrt(dx*dx + dy*dy);
}

//For Fo
static Fo_Group mergeGroups(const Fo_Group& g1, const Fo_Group& g2, int newGroupId) {
    Fo_Group merged(newGroupId);
    for (const auto& fo : g1.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    for (const auto& fo : g2.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    return merged;
}

//For Tec
static Tec_Group mergeGroups(const Tec_Group& g1, const Tec_Group& g2, int newGroupId) {
    Tec_Group merged(newGroupId);
    for (const auto& fo : g1.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    for (const auto& fo : g2.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    return merged;
}

//For Fo
void clusterFo_Group(vector<Fo_Group>& groups, int est_threshold, int lst_threshold, int max_group_size) {
    if (groups.size() <= 1) return;

    bool merged = true;
    int nextGroupId = 1000;


    while (merged && groups.size() > 1) {
        merged = false;
        double minDist = numeric_limits<double>::max();
        int bestA = -1, bestB = -1;

        // Find the two closest groups
        for (size_t i = 0; i < groups.size(); ++i) {
            for (size_t j = i + 1; j < groups.size(); ++j) {
                double dist = groupDistance(groups[i], groups[j]);
                if (dist < minDist) {
                    minDist = dist;
                    bestA = (int)i;
                    bestB = (int)j;
                }
            }
        }

        if (bestA == -1 || bestB == -1) {
            break;
        }

        Fo_Group candidate = mergeGroups(groups[bestA], groups[bestB], nextGroupId++);
        MBR candidateMBR;
        createMBR(candidate, candidateMBR);

        bool thresholdOK = !exceedsThreshold(candidateMBR, est_threshold, lst_threshold);
        bool sizeOK = (int)candidate.getFlexOffers().size() <= max_group_size;

        if (thresholdOK && sizeOK) {
            if (bestA > bestB) swap(bestA, bestB);
            groups.erase(groups.begin() + bestB);
            groups.erase(groups.begin() + bestA);
            groups.push_back(candidate);
            merged = true;
        } else {
            merged = false;
        }
    }

}

//For tec
void clusterFo_Group(vector<Tec_Group>& groups, int est_threshold, int lst_threshold, int max_group_size) {
    if (groups.size() <= 1) return;

    bool merged = true;
    int nextGroupId = 1000;

    while (merged && groups.size() > 1) {
        merged = false;
        double minDist = numeric_limits<double>::max();
        int bestA = -1, bestB = -1;

        // Find the two closest groups
        for (size_t i = 0; i < groups.size(); ++i) {
            for (size_t j = i + 1; j < groups.size(); ++j) {
                double dist = groupDistance(groups[i], groups[j]);
                if (dist < minDist) {
                    minDist = dist;
                    bestA = (int)i;
                    bestB = (int)j;
                }
            }
        }

        if (bestA == -1 || bestB == -1) {
            break;
        }


        Tec_Group candidate = mergeGroups(groups[bestA], groups[bestB], nextGroupId++);
        MBR candidateMBR;
        createMBR(candidate, candidateMBR);

        bool thresholdOK = !exceedsThreshold(candidateMBR, est_threshold, lst_threshold);
        bool sizeOK = (int)candidate.getFlexOffers().size() <= max_group_size;

        if (thresholdOK && sizeOK) {
            if (bestA > bestB) swap(bestA, bestB);
            groups.erase(groups.begin() + bestB);
            groups.erase(groups.begin() + bestA);
            groups.push_back(candidate);
            merged = true;
        } else {
            merged = false;
        }
    }

}

vector<double> readSpotPricesFromCSV(const string& filename) {
    vector<double> spotPrices;
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error opening file for reading: " << filename << endl;
        return spotPrices;
    }

    string line;
    // Skip header line
    if (!getline(inFile, line)) {
        cerr << "Error reading header from file: " << filename << endl;
        return spotPrices;
    }

    // Read data lines
    while (getline(inFile, line)) {
        try {
            spotPrices.push_back(stod(line));
        } catch (const invalid_argument& e) {
            cerr << "Invalid data in file: " << line << endl;
        }
    }

    inFile.close();
    return spotPrices;
}

tuple<vector<double>, vector<double>, vector<double>, vector<double>> getFRCprices(const string& filename) {
    vector<double> UpPrices, DownPrices, UpVolume, DownVolume;
    ifstream file(filename);

    // Read each line of the CSV
    string line;
    if (getline(file, line)) {
    }

    while (getline(file, line)) {
        stringstream ss(line);
        string value1, value2, value3, value4;

        // Read the two comma-separated values
        if (getline(ss, value1, ',') && getline(ss, value2, ','), getline(ss, value3, ',') && getline(ss, value4, ',')) {
            UpPrices.push_back(stod(value1));
            DownPrices.push_back(stod(value2));
            UpVolume.push_back(stod(value3));
            DownVolume.push_back(stod(value4));
        } else {
            cerr << "Error: Malformed line in CSV: " << line << endl;
        }
    }

    file.close();
    return {UpPrices, DownPrices, UpVolume, DownVolume};
}


// Helper to trim quotes from a string
string trimQuotes(const string& str) {
    return (str.size() >= 2 && str.front() == '"' && str.back() == '"') ? str.substr(1, str.size() - 2) : str;
}

// Convert a date-time string to time_t
time_t parseDateTime(const string& datetime) {
    istringstream ss(datetime);
    tm timeinfo = {};
    ss >> get_time(&timeinfo, "%a, %d %b %Y %H:%M:%S GMT");
    if (ss.fail()) throw runtime_error("Invalid date-time format: " + datetime);
    return timegm(&timeinfo);
}

// Calculate duration in hours
int calculateDuration(time_t start, time_t end) {
    return max(0, static_cast<int>(difftime(end, start) / 3600));
}

// Calculate min and max power
pair<double, double> calculatePowerRange(double kWhDelivered, int duration) {
    if (duration <= 0) return {0.0, 0.0};
    double avgPower = kWhDelivered / duration;
    return {avgPower * 0.8, avgPower * 1.2};
}

// Parse CSV line into fields
vector<string> parseCSVLine(const string& line) {
    vector<string> fields;
    string field;
    bool inQuotes = false;

    for (char ch : line) {
        if (ch == '"') inQuotes = !inQuotes;
        else if (ch == ',' && !inQuotes) {
            fields.push_back(trimQuotes(field));
            field.clear();
        } else field += ch;
    }
    fields.push_back(trimQuotes(field));
    return fields;
}

time_t roundToNearestHour(time_t raw_time) {
    tm *timeinfo = gmtime(&raw_time);
    timeinfo->tm_min = 0;
    timeinfo->tm_sec = 0;

    return timegm(timeinfo); // Convert back to time_t
}

vector<variant<Flexoffer, Tec_flexoffer>> parseEVDataToFlexOffers(const string& filename, int type) {
    ifstream file(filename);
    if (!file.is_open()) throw runtime_error("Error: Could not open file " + filename);

    vector<variant<Flexoffer, Tec_flexoffer>> flexOffers;
    string line;
    getline(file, line);

    int offerID = 1;
    while (getline(file, line)) {
        if (line.empty()) continue;

        auto fields = parseCSVLine(line);
        if (fields.size() < 6) {
            cerr << "Skipping invalid line: " << line << endl;
            continue;
        }

        // Parse times and kWhDelivered
        time_t connectionTime = parseDateTime(fields[2]);
        time_t doneChargingTime = fields[4].empty() ? parseDateTime(fields[3]) : parseDateTime(fields[4]);
        double kWhDelivered = fields[5].empty() ? 0.0 : stod(fields[5]);

        // Round connection and end times
        connectionTime = roundToNearestHour(connectionTime);
        doneChargingTime = roundToNearestHour(doneChargingTime);

        // Calculate required charging duration in hours
        double requiredHours = ceil(kWhDelivered / 7.2); // Assuming an average charging power of 7.2 kW
        int duration = static_cast<int>(requiredHours); // Duration in hours
        time_t durationInSeconds = static_cast<time_t>(requiredHours * 3600);

        // Calculate latestStartTime
        time_t latestStartTime = doneChargingTime - durationInSeconds;
        if (latestStartTime < connectionTime) {
            latestStartTime = connectionTime; // Ensure latestStartTime respects connectionTime
        }

        // Calculate profile for the duration
        auto [minPower, maxPower] = calculatePowerRange(kWhDelivered / duration, duration);
        vector<TimeSlice> profile(duration, {minPower, maxPower});

        // Calculate actual min and max energy for the profile
        double actualMinEnergy = minPower * duration;
        double actualMaxEnergy = maxPower * duration;

        // Calculate TEC constraints
        double totalMinEnergy = actualMinEnergy * 1.0; // Example: 90% of actual minimum energy
        double totalMaxEnergy = actualMaxEnergy * 5.0; // Example: 110% of actual maximum energy

        // Create Flexoffer or Tec_flexoffer based on type
        if (type == 0) { // Normal Flexoffer
            flexOffers.emplace_back(Flexoffer(offerID++, connectionTime, latestStartTime, doneChargingTime, profile, duration));
        } else if (type == 1) { // TEC Flexoffer
            flexOffers.emplace_back(Tec_flexoffer(totalMinEnergy, totalMaxEnergy, offerID++, connectionTime, latestStartTime, doneChargingTime, profile, duration));
        }
    }

    return flexOffers;
}


std::vector<DFO> parseEVDataToDFO(const std::string &filename, int numsamples = 4) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open file " + filename);
    }

    std::vector<DFO> dfos; // Collect one DFO per EV session

    // Skip header line
    std::string line;
    if (!std::getline(file, line)) {
        return dfos; // Empty file
    }

    int dfo_id = 1; // Increment for each EV session
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        auto fields = parseCSVLine(line);
        if (fields.size() < 6) {
            std::cerr << "[parseEVDataToDFO] Skipping invalid line:\n" << line << std::endl;
            continue;
        }

        // Parse connection and doneCharging times
        time_t connectionTime = parseDateTime(fields[2]);
        time_t doneChargingTime = parseDateTime(fields[3]);
        time_t disconnectTime = parseDateTime(fields[4]);
        connectionTime = roundToNearestHour(connectionTime);
        doneChargingTime = roundToNearestHour(doneChargingTime);
        disconnectTime = roundToNearestHour(disconnectTime);

        // Parse kWhDelivered
        double kWhDelivered = fields[5].empty() ? 0.0 : std::stod(fields[5]);
        if (kWhDelivered <= 0.0) {
            std::cerr << "Warning: zero or negative kWh, skipping line.\n";
            continue;
        }

        // Compute charging duration and charging rate
        double diff_sec = std::difftime(doneChargingTime, connectionTime);
        if (diff_sec <= 0) {
            std::cerr << "Warning: doneChargingTime <= connectionTime, skipping.\n";
            continue;
        }
        int charging_hours = static_cast<int>(std::ceil(diff_sec / 3600.0));
        double charging_rate = kWhDelivered / charging_hours;

        // Create 24 time slices
        const int total_time_slices = 24;
        std::vector<double> min_prev(total_time_slices + 1, 0.0);
        std::vector<double> max_prev(total_time_slices + 1, 0.0);

        // Scale max_prev using the full charging rate from the connection time
        double cumulative_energy = 0.0;
        for (int i = connectionTime / 3600; i < doneChargingTime / 3600 && cumulative_energy < kWhDelivered; ++i) {
            cumulative_energy += charging_rate;
            if (cumulative_energy > kWhDelivered) {
                cumulative_energy = kWhDelivered;
            }
            max_prev[i + 1] = cumulative_energy;
        }

        // Scale min_prev starting from the last charging hour
        cumulative_energy = kWhDelivered;
        for (int i = doneChargingTime / 3600; i >= connectionTime / 3600 && cumulative_energy > 0; --i) {
            min_prev[i + 1] = cumulative_energy;
            cumulative_energy -= charging_rate;
            if (cumulative_energy < 0) {
                cumulative_energy = 0;
            }
        }

        // For time slices after disconnectTime, set min/max to kWhDelivered
        for (int i = disconnectTime / 3600 + 1; i <= total_time_slices; ++i) {
            min_prev[i] = kWhDelivered;
            max_prev[i] = kWhDelivered;
        }

        // Create the DFO
        DFO myDFO(dfo_id++, min_prev, max_prev, numsamples);

        // Generate dependency polygons
        myDFO.generate_dependency_polygons();

        // Store the DFO
        dfos.push_back(myDFO);
    }

    return dfos;
}

//For FO
vector<AggregatedFlexOffer> nToMAggregation(vector<Flexoffer> &allFlexoffers, 
                                            int est_threshold, 
                                            int lst_threshold, 
                                            int max_group_size,
                                            Alignments align, 
                                            int startFo_GroupId=1)
{
    vector<Fo_Group> groups;
    int groupId = startFo_GroupId;
    for (const auto &fo : allFlexoffers) {
        Fo_Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    clusterFo_Group(groups, est_threshold, lst_threshold, max_group_size);

    vector<AggregatedFlexOffer> finalAggregates;
    finalAggregates.reserve(groups.size());
    for (auto &g : groups) {
        finalAggregates.push_back(g.createAggregatedOffer(align));
    }

    // 7) Return the final AFOs
    return finalAggregates;
}
vector<AggregatedFlexOffer> nToMAggregation(vector<Flexoffer> &allFlexoffers, 
                                            int est_threshold, 
                                            int lst_threshold, 
                                            int max_group_size,
                                            Alignments align, 
                                            const vector<double> &spotPrices,
                                            int startFo_GroupId=1)
{
    vector<Fo_Group> groups;
    int groupId = startFo_GroupId;
    for (const auto &fo : allFlexoffers) {
        Fo_Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    clusterFo_Group(groups, est_threshold, lst_threshold, max_group_size);

    vector<AggregatedFlexOffer> finalAggregates;
    finalAggregates.reserve(groups.size());
    for (auto &g : groups) {
        finalAggregates.push_back(g.createAggregatedOffer(align, spotPrices));
    }

    return finalAggregates;
}

//For tec
vector<AggregatedFlexOffer> nToMAggregation(vector<Tec_flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align,int startFo_GroupId=1){
    vector<Tec_Group> groups;
    int groupId = startFo_GroupId;
    for (const auto &fo : allFlexoffers) {
        Tec_Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    clusterFo_Group(groups, est_threshold, lst_threshold, max_group_size);

    vector<AggregatedFlexOffer> finalAggregates;
    finalAggregates.reserve(groups.size());
    for (auto &g : groups) {
        finalAggregates.push_back(g.createAggregatedOffer(align));
    }

    return finalAggregates;
}
vector<AggregatedFlexOffer> nToMAggregation(vector<Tec_flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align,const vector<double> &spotPrices,int startFo_GroupId=1){
    vector<Tec_Group> groups;
    int groupId = startFo_GroupId;
    for (const auto &fo : allFlexoffers) {
        Tec_Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    clusterFo_Group(groups, est_threshold, lst_threshold, max_group_size);

    vector<AggregatedFlexOffer> finalAggregates;
    finalAggregates.reserve(groups.size());
    for (auto &g : groups) {
        finalAggregates.push_back(g.createAggregatedOffer(align, spotPrices));
    }

    return finalAggregates;
}





// static vector<double> buildDailySchedule(const AggregatedFlexOffer &afo) {
//     const int DAY_HOURS = 24;
//     vector<double> daySched(DAY_HOURS, 0.0);
//     int startHour = afo.get_aggregated_earliest_hour();
//     int duration  = afo.get_duration();
//     auto aggAlloc = afo.get_scheduled_allocation();

//     for (int i = 0; i < duration; i++) {
//         int idx = startHour + i;
//         if (idx >= 0 && idx < DAY_HOURS) {
//             daySched[idx] += aggAlloc[i];
//         }
//     }

//     return daySched;
// }


// void dumpMetricsToCSV(const string& filename, const vector<string>& headers, const vector<vector<double>>& data) {
//     ofstream file(filename);
//     if (!file.is_open()) {
//         cerr << "Failed to open file: " << filename << endl;
//         return;
//     }

//     // Write headers
//     for (size_t i = 0; i < headers.size(); ++i) {
//         file << headers[i];
//         if (i < headers.size() - 1) file << ",";
//     }
//     file << "\n";

//     // Write data row by row
//     size_t rows = data.empty() ? 0 : data[0].size();
//     for (size_t row = 0; row < rows; ++row) {
//         for (size_t col = 0; col < data.size(); ++col) {
//             file << data[col][row];
//             if (col < data.size() - 1) file << ",";
//         }
//         file << "\n";
//     }

//     file.close();
//     cout << "Metrics dumped to " << filename << endl;
// }

// void prepareAndDumpMetrics(const vector<double> &spotPrices,
//                            const vector<AggregatedFlexOffer> &afos,
//                            const string &csvFilePath,
//                            const string &pythonScriptPath)
// {
//     int DAY_HOURS = spotPrices.size();
//     vector<double> withFlex(DAY_HOURS, 0.0);
//     vector<double> noFlexPower(24, 0.0);


//     for (auto &afo : afos) {
//         vector<double> daySched = buildDailySchedule(afo); // 24-hr array
//         for (int h = 0; h < DAY_HOURS; h++) {
//             withFlex[h] += daySched[h];
//         }
//     }

//     ofstream file(csvFilePath);
//     if (!file.is_open()) {
//         cerr << "Failed to open file: " << csvFilePath << endl;
//         return;
//     }

//     // Write header
//     file << "Hour,SpotPrice,WithFlexPower,WithFlexCost,";

//     // Fill rows
//     for (int h = 0; h < DAY_HOURS; h++) {
//         double price = spotPrices[h];
//         double flexPwr = withFlex[h];
//         double flexCost = flexPwr * price;

//         file << h << "," << price << "," << flexPwr << "," << flexCost << "\n";
//     }

//     file.close();
//     cout << "Metrics dumped to " << csvFilePath << endl;

//     string python_command = "python3 " + pythonScriptPath;
//     int result = system(python_command.c_str());
//     if (result != 0) {
//         cerr << "Failed to execute Python script: " << python_command << endl;
//     }
// }


// void dumpFCRDataToCSV(const vector<vector<double>> &powerVars,
//                    const vector<vector<double>> &upVars,
//                    const vector<vector<double>> &downVars,
//                    double totalRevenue,
//                    const string &filename)
// {
//     ofstream outFile(filename);
//     if (!outFile.is_open()) {
//         cerr << "Error opening file: " << filename << endl;
//         return;
//     }

//     // Write a header row
//     outFile << "AFO,Hour,Power,UpReg,DownReg\n";

//     // powerVars.size() should be the number of AggregatedFlexOffers
//     int A = static_cast<int>(powerVars.size());
//     for (int a = 0; a < A; a++) {
//         // For each AFO 'a', we assume the same indexing in upVars[a] and downVars[a]
//         int duration = static_cast<int>(powerVars[a].size());
//         for (int t = 0; t < duration; t++) {
//             outFile << a << "," << t << ","<< powerVars[a][t] << ","<< upVars[a][t] << ","<< downVars[a][t] << "\n";
//         }
//     }

//     // Optionally, include the total revenue on a separate line at the bottom
//     outFile << "\nTotalRevenue," << totalRevenue << "\n";
//     outFile.close();

//     string pythonPath = "../visuals/plot_fcr.py";

//     string python_command = "python3 "+ pythonPath;
//     int result = system(python_command.c_str());
//     if (result != 0) {
//         cerr << "Failed to execute Python script: " << python_command << endl;
//     }
// }



// static int hourOfDay(time_t t) {
//     struct tm* tmInfo = localtime(&t);
//     return tmInfo->tm_hour;
// }

// void dumpSolverAndDisaggResults(vector<AggregatedFlexOffer> &afos, vector<double> &spotPrices, const string &aggCsvPath, const string &disCsvPath) {
//     ofstream outFile(aggCsvPath);
//     outFile << "AggregatorID,Hour,ScheduledPower,Cost\n";

//     for (size_t a = 0; a < afos.size(); a++) {
//         const auto &afo = afos[a];
//         int duration = afo.get_duration();
//         auto schedule = afo.get_scheduled_allocation();

//         time_t baseTime = afo.get_aggregated_earliest(); 
        
//         for (int h = 0; h < duration; h++) {
//             // convert to absolute time
//             time_t currentSlice = baseTime + (h * 3600); 
//             int absHour = hourOfDay(currentSlice);
//             double power = schedule[h];
//             double price =  spotPrices[absHour];
//             double cost  = power * price;

//             outFile << afos[a].get_id() << ","
//                     << absHour << ","
//                     << power   << ","
//                     << cost    << "\n";
//         }
//     }
//     outFile.close();
//     cout << "Aggregator-level results written to " << aggCsvPath << "\n";

//     ofstream outFile2(disCsvPath);
//     outFile2 << "AggregatorID,FlexOfferID,Hour,ScheduledPower,Cost\n";

//     for (size_t a = 0; a < afos.size(); a++) {

//         vector<Flexoffer> originalFOs = afos[a].disaggregate_to_flexoffers();

//         for (auto &fo : originalFOs) {
//             int fo_duration = fo.get_duration();
//             auto fo_sched   = fo.get_scheduled_allocation();
//             time_t foStart = fo.get_scheduled_start_time();

//             for (int h = 0; h < fo_duration; h++) {
//                 // actual time = foStart + h*3600
//                 time_t currentSlice = foStart + (h * 3600);
//                 int absHour = hourOfDay(currentSlice);
//                 double power = fo_sched[h];
//                 double price = spotPrices[absHour];
//                 double cost  = power * price;

//                 outFile2 << afos[a].get_id()  << ","
//                         << fo.get_offer_id()  << ","
//                         << absHour            << ","
//                         << power              << ","
//                         << cost               << "\n";
//             }
//         }
//     }
//     outFile2.close();
// }