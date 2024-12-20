#include "../include/helperfunctions.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>
#include <limits>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <utility>

using namespace std;

void createMBR(const Group& group, MBR& mbr) {
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

static double groupDistance(const Group& g1, const Group& g2) {
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

static Group mergeGroups(const Group& g1, const Group& g2, int newGroupId) {
    Group merged(newGroupId);
    for (const auto& fo : g1.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    for (const auto& fo : g2.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    return merged;
}

void clusterGroup(vector<Group>& groups, int est_threshold, int lst_threshold, int max_group_size) {
    if (groups.size() <= 1) return;

    bool merged = true;
    int nextGroupId = 1000;

    cout << "[DEBUG] Starting bottom-up hierarchical clustering...\n";
    cout << "[DEBUG] Initial number of groups: " << groups.size() << "\n";

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
            cout << "[DEBUG] No pairs found for merging.\n";
            break;
        }

        cout << "[DEBUG] Closest groups to merge: Group " << groups[bestA].getGroupId()
                  << " and Group " << groups[bestB].getGroupId() << " with distance " << minDist << "\n";

        Group candidate = mergeGroups(groups[bestA], groups[bestB], nextGroupId++);
        MBR candidateMBR;
        createMBR(candidate, candidateMBR);

        bool thresholdOK = !exceedsThreshold(candidateMBR, est_threshold, lst_threshold);
        bool sizeOK = (int)candidate.getFlexOffers().size() <= max_group_size;

        if (thresholdOK && sizeOK) {
            cout << "[DEBUG] Merging groups " << groups[bestA].getGroupId() << " and " << groups[bestB].getGroupId() << " into new Group " << candidate.getGroupId() << "\n";
            if (bestA > bestB) swap(bestA, bestB);
            groups.erase(groups.begin() + bestB);
            groups.erase(groups.begin() + bestA);
            groups.push_back(candidate);
            merged = true;
        } else {
            cout << "[DEBUG] Cannot merge these two groups due to " 
                      << (thresholdOK ? "" : "threshold violation ") 
                      << (thresholdOK && !sizeOK ? "and " : "") 
                      << (!sizeOK ? "max group size exceeded" : "")
                      << ". Stopping.\n";
            merged = false;
        }
    }

    cout << "[DEBUG] Clustering complete. Final number of groups: " << groups.size() << "\n";
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

// Parse EV data into Flexoffer objects
vector<Flexoffer> parseEVDataToFlexOffers(const string& filename) {
    ifstream file(filename);
    vector<Flexoffer> flexOffers;
    string line;
    getline(file, line); 

    int offerID = 1;
    while (getline(file, line)) {
        if (line.empty()) continue;

        auto fields = parseCSVLine(line);

        // Parse times and kWhDelivered
        time_t connectionTime = parseDateTime(fields[2]);
        time_t disconnectTime = parseDateTime(fields[3]);
        time_t doneChargingTime = fields[4].empty() ? disconnectTime : parseDateTime(fields[4]);
        double kWhDelivered = fields[5].empty() ? 0.0 : stod(fields[5]);

        // removed minutes and seconds
        connectionTime = roundToNearestHour(connectionTime);
        doneChargingTime = roundToNearestHour(doneChargingTime);

        // Calculate duration and power range
        int duration = calculateDuration(connectionTime, doneChargingTime);
        auto [minPower, maxPower] = calculatePowerRange(kWhDelivered, duration);

        time_t latestStartTime = doneChargingTime - static_cast<time_t>(duration * 3600);
        latestStartTime = roundToNearestHour(latestStartTime);

        // Create profile and Flexoffer
        vector<TimeSlice> profile(duration, {minPower, maxPower});
        flexOffers.emplace_back(offerID++, connectionTime, latestStartTime, doneChargingTime, profile, duration);
    }
    return flexOffers;
}
