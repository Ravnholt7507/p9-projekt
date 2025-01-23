#include "../include/helperfunctions.h"
#include "../include/solver.h"
#include <algorithm>
#include <fstream>
#include <chrono>
#include <fstream>
#include <chrono>  
#include <iostream>
#include <map>
#include <sstream>
#include <cmath>
#include <limits>
#include <iostream>
#include <queue>
#include <set>
#include <cstdlib>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <utility>


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

    return timegm(timeinfo); 
}

static bool isSameLocalDay(time_t t1, time_t t2)
{
    // If you prefer gmtime, do so consistently
    tm* local1 = localtime(&t1);
    tm* local2 = localtime(&t2);

    return (local1->tm_year == local2->tm_year &&
            local1->tm_mon  == local2->tm_mon  &&
            local1->tm_mday == local2->tm_mday);
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

        time_t connectionTime = parseDateTime(fields[2]);
        time_t doneChargingTime = fields[4].empty() ? parseDateTime(fields[3]) : parseDateTime(fields[4]);
        double kWhDelivered = fields[5].empty() ? 0.0 : stod(fields[5]);

        connectionTime = roundToNearestHour(connectionTime);
        doneChargingTime = roundToNearestHour(doneChargingTime);

        if (!isSameLocalDay(connectionTime, doneChargingTime)) {
            continue;  
        }

        double requiredHours = ceil(kWhDelivered / 7.2);
        int duration = static_cast<int>(requiredHours);
        time_t durationInSeconds = static_cast<time_t>(requiredHours * 3600);

        time_t latestStartTime = doneChargingTime - durationInSeconds;

        if (latestStartTime < connectionTime) {
            continue;
            //latestStartTime = connectionTime;
        }

        time_t end_time = latestStartTime + durationInSeconds;
        
        if (!isSameLocalDay(connectionTime, end_time)) {
            continue;
        }


        auto [minPower, maxPower] = calculatePowerRange(kWhDelivered / duration, duration);
        vector<TimeSlice> profile(duration, {minPower, maxPower});

        double actualMinEnergy = minPower * duration;
        double actualMaxEnergy = maxPower * duration;

        double totalMinEnergy = actualMinEnergy * 1.2;
        double totalMaxEnergy = actualMaxEnergy * 1.5;

        if (type == 0) {
            flexOffers.emplace_back(Flexoffer(offerID++, connectionTime, latestStartTime, doneChargingTime, profile, duration));
        } else if (type == 1) {
            flexOffers.emplace_back(Tec_flexoffer(totalMinEnergy, totalMaxEnergy, offerID++, connectionTime, latestStartTime, doneChargingTime, profile, duration));
        }
    }

    return flexOffers;
}



int parseDateTimeToHour(const string &dateTimeStr) {
    struct tm tm = {};
    if (strptime(dateTimeStr.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tm) == nullptr) {
        throw runtime_error("Failed to parse date/time: " + dateTimeStr);
    }
    return tm.tm_hour; // Extract the hour of the day (0–23)
}


vector<DFO> parseEVDataToDFO(const string &filename, int numsamples = 4) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Error: Could not open file " + filename);
    }

    vector<DFO> dfos; // Collect one DFO per EV session

    // Skip header line
    string line;
    if (!getline(file, line)) {
        return dfos; // Empty file
    }

    int dfo_id = 1; // Increment for each EV session
    while (getline(file, line)) {
        if (line.empty()) continue;

        auto fields = parseCSVLine(line);
        if (fields.size() < 6) {
            cerr << "[parseEVDataToDFO] Skipping invalid line:\n" << line << endl;
            continue;
        }

        // Parse times as hours of the day
        int connectionHour, disconnectHour, doneChargingHour;
        try {
            connectionHour = parseDateTimeToHour(fields[2]);
            disconnectHour = parseDateTimeToHour(fields[3]);
            doneChargingHour = parseDateTimeToHour(fields[4]);
        } catch (const exception &e) {
            cerr << "Error parsing date/time: " << e.what() << " in line: " << line << endl;
            continue;
        }


        // Parse kWhDelivered
        double kWhDelivered = 0.0;
        try {
            kWhDelivered = stod(fields[5]) * 1.2;
        } catch (const exception &e) {
            cerr << "Error parsing kWhDelivered: " << e.what() << " in line: " << line << endl;
            continue;
        }

        if (kWhDelivered <= 0.0) {
            cerr << "Warning: zero or negative kWhDelivered, skipping line." << endl;
            continue;
        }

        if (disconnectHour == 23 || disconnectHour == 24 || doneChargingHour == 23 || doneChargingHour == 24 ||  kWhDelivered < 1){
            continue;
        }

        if (!isSameLocalDay(connectionHour, disconnectHour) || !isSameLocalDay(connectionHour, doneChargingHour)) {
            continue;
        }

        // Compute charging duration and charging rate
        int charging_hours = doneChargingHour - connectionHour;
        if (charging_hours <= 0 || doneChargingHour >= disconnectHour) {
            continue;
        }
        double charging_rate = kWhDelivered / charging_hours;

        // Create 24 time slices
        const int total_time_slices = 24;
        vector<double> min_prev(total_time_slices, 0.0);
        vector<double> max_prev(total_time_slices, 0.0);

        // Scale max_prev using the full charging rate from the connection hour
        double cumulative_energy = 0.0;
        for (int i = connectionHour; i <= doneChargingHour; ++i) {
            cumulative_energy += charging_rate;
            if (cumulative_energy > kWhDelivered) {
                cumulative_energy = kWhDelivered;
            }
            //cout <<"i+1: " <<i+1 << "\n";  
            max_prev[i+1] = cumulative_energy;
        }

        // Scale min_prev starting from the last charging hour
        cumulative_energy = kWhDelivered;
        for (int i = doneChargingHour; i >= connectionHour && cumulative_energy > 0; --i) {
            min_prev[i+1] = cumulative_energy;
            cumulative_energy -= charging_rate;
            if (cumulative_energy < 0) {
                cumulative_energy = 0;
            }
        }


        for (int i = doneChargingHour + 1; i <= disconnectHour; i++) {
            min_prev[i] = kWhDelivered;
            max_prev[i] = kWhDelivered;
        }

        for (int i = disconnectHour + 1; i < total_time_slices; ++i) {
            min_prev[i] = kWhDelivered;
            max_prev[i] = kWhDelivered;
        }

        // Create the DFO
        DFO myDFO(dfo_id++, min_prev, max_prev, numsamples);
        
        myDFO.generate_dependency_polygons();


        // Store the DFO
        dfos.push_back(myDFO);
    }

    return dfos;
}