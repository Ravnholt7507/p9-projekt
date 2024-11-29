#include <vector>
#include <set>
#include <algorithm>

//#include "../include/ChangesList.h"
#include "../include/helperfunctions.h"
#include "../include/flexoffer.h"
#include "../include/group.h"

using namespace std;

const int maxGroupSize = 2;

pair<vector<int>, vector<int>> calculateMBR(vector<Flexoffer> &offers) 
{
    // Initialize MBR values
    int minEarliestStartTime = numeric_limits<int>::max();
    int maxEarliestStartTime = numeric_limits<int>::min();
    int minLatestStartTime = numeric_limits<int>::max();
    int maxLatestStartTime = numeric_limits<int>::min();

    // Iterate over the set of offer IDs
    for (Flexoffer& f : offers) {
        // Convert time_t to integer
        time_t offer_est = f.get_est();
        time_t offer_lst = f.get_lst();
        int est = localtime(&offer_est)->tm_hour;
        int lst = localtime(&offer_lst)->tm_hour;

        // Update MBR values
        minEarliestStartTime = min(minEarliestStartTime, est);
        maxEarliestStartTime = max(maxEarliestStartTime, est);
        minLatestStartTime = min(minLatestStartTime, lst);
        maxLatestStartTime = max(maxLatestStartTime, lst);
    }

    // Prepare the result as a pair of vectors
    vector<int> minValues = {minEarliestStartTime, minLatestStartTime};
    vector<int> maxValues = {maxEarliestStartTime, maxLatestStartTime};

    return {minValues, maxValues};
}

// Check if MBR exceeds thresholds
bool doesMBRExceedThreshold(
    const pair<vector<int>, vector<int>>& mbr, 
    const vector<int>& thresholds
) {
    for (size_t i = 0; i < mbr.first.size(); ++i) {
        if (mbr.second[i] - mbr.first[i] > thresholds[i]) {
            return true;
        }
    }
    return false;
}



// Bin packing algorithm
vector<Group> binPackGroup(
    const Group& group, 
    int max_size, 
    GroupHash& group_hash
   // ChangesList& change_list
) {
    vector<Group> bins;
    vector<int> offer_ids(group.flexOfferIDs.begin(), group.flexOfferIDs.end());

    size_t index = 0;
    while (index < offer_ids.size()) {
        Group bin(group_hash.generateUniqueGroupID());
        bin.cells = group.cells; // All bins share the same cells

        for (int i = 0; i < max_size && index < offer_ids.size(); ++i, ++index) {
            bin.flexOfferIDs.insert(offer_ids[index]);
        }
        bins.push_back(bin);
    }
    return bins;
}


// Hierarchical clustering with MBR checks
vector<Group> clusterHierarch(Group& group, GroupHash& gh, vector<int>& thresholds, vector<Flexoffer>& flexOffers) {
    vector<Group> clusters;
    auto cells = group.cells;

    cout << cells.size();

    //dividing group into multiple groups
    for (const auto& cell : cells) {
        int group_id = gh.generateUniqueGroupID();
        Group group(group_id);
        group.cells.insert(cell);
        if (gh.grid.hasCell(cell)) {
            const auto& ids = gh.grid.getFlexOffersInCell(cell);
            group.flexOfferIDs.insert(ids.begin(), ids.end());
        }
        clusters.push_back(group);
    }

    // Joining groups together
    bool merged = false;
    vector<Group> new_clusters;
    while (clusters.size() > 1) {
        for (size_t i = 0; i < clusters.size(); ++i) {
            if (i < clusters.size() - 1) {
                Group merged_group;
                merged_group.id = gh.generateUniqueGroupID();
                merged_group.cells.insert(clusters[i].cells.begin(), clusters[i].cells.end());
                merged_group.cells.insert(clusters[i + 1].cells.begin(), clusters[i + 1].cells.end());
                merged_group.flexOfferIDs.insert(clusters[i].flexOfferIDs.begin(), clusters[i].flexOfferIDs.end());
                merged_group.flexOfferIDs.insert(clusters[i + 1].flexOfferIDs.begin(), clusters[i + 1].flexOfferIDs.end());

                vector<Flexoffer> merged_flexoffers = getFlexOffersById(merged_group.flexOfferIDs, flexOffers);

                auto mbr = calculateMBR(merged_flexoffers);
                if (!doesMBRExceedThreshold(mbr, thresholds)) {
                    new_clusters.push_back(merged_group);
                    ++i;
                    merged = true;
                } else {
                    new_clusters.push_back(clusters[i]);
                }
            } else {
                new_clusters.push_back(clusters[i]);
            }
        }
        if (!merged) break;
        clusters = move(new_clusters);
    }

    return new_clusters;
}


// Optimize group to meet thresholds or size constraints
void optimizeGroup(int group_id, GroupHash &gh, vector<int> thresholds, vector<Flexoffer> &flexOffers) {
    auto it = gh.groups.find(group_id);
    if (it == gh.groups.end()) return; // Group not found

    Group group_copy = it->second; // Copy for processing
    vector<Flexoffer> FlexOffersInGroup = getFlexOffersById(group_copy.flexOfferIDs, flexOffers); //get FOs in group
    auto mbr = calculateMBR(FlexOffersInGroup);
    if (doesMBRExceedThreshold(mbr, thresholds)) { //|| group_copy.flexOfferIDs.size() > maxGroupSize
        cout << "\n" << "exceeded" << "\n";
        cout << "groupID: " << group_copy.id << "\n";
        for (const auto& id : group_copy.flexOfferIDs) {
            cout << "FO_ID: " << id << "\n" ;
        }

        gh.removeGroup(group_id);

        auto new_clusters = clusterHierarch(group_copy, gh, thresholds, flexOffers);
        for (Group& new_group : new_clusters) {
            gh.groups[new_group.id] = new_group;
            for (const auto& cell : new_group.cells) {
                gh.cellToGroupMap[cell] = new_group.id;
            }
        }
    }
    cout << "groupID: " << group_copy.id << "\n";
    for (const auto& id : group_copy.flexOfferIDs) {
        cout << "FO_ID: " << id << "\n" ;
    }
}


// Handle delta updates for FlexOffers
void deltaProcess(
    const Flexoffer& offer, 
    char action, 
    GroupHash& group_hash
) {
    if (action == '+') {
        group_hash.addFlexOffer(offer);
    } else if (action == '-') {
        group_hash.removeFlexOffer(offer);
    }
}
