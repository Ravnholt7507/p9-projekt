#include <vector>
#include <set>
#include <algorithm>

#include "../include/ChangesList.h"
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
    GroupHash& group_hash, 
    ChangesList& change_list
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

        // Register the addition of the new bin
        change_list.registerChange(bin.id, '+', vector<int>(bin.flexOfferIDs.begin(), bin.flexOfferIDs.end()));
    }
    return bins;
}


/*

// Hierarchical clustering with MBR checks
vector<Group> clusterHierarch(
    const set<Cell>& cells, 
    Grid& grid, 
    GroupHash& group_hash, 
    const vector<int>& thresholds, 
    const unordered_map<int, Flexoffer>& flexOffers
) {
    vector<Group> clusters;
    for (const auto& cell : cells) {
        int group_id = group_hash.generateUniqueGroupID();
        Group group(group_id);
        group.cells.insert(cell);
        if (grid.hasCell(cell)) {
            const auto& ids = grid.getFlexOffersInCell(cell);
            group.flexOfferIDs.insert(ids.begin(), ids.end());
        }
        clusters.push_back(group);
    }

    while (clusters.size() > 1) {
        bool merged = false;
        vector<Group> new_clusters;

        for (size_t i = 0; i < clusters.size(); ++i) {
            if (i < clusters.size() - 1) {
                Group merged_group;
                merged_group.id = group_hash.generateUniqueGroupID();
                merged_group.cells.insert(clusters[i].cells.begin(), clusters[i].cells.end());
                merged_group.cells.insert(clusters[i + 1].cells.begin(), clusters[i + 1].cells.end());
                merged_group.flexOfferIDs.insert(clusters[i].flexOfferIDs.begin(), clusters[i].flexOfferIDs.end());
                merged_group.flexOfferIDs.insert(clusters[i + 1].flexOfferIDs.begin(), clusters[i + 1].flexOfferIDs.end());

                auto mbr = calculateMBR(merged_group.flexOfferIDs, grid.featureExtractors, flexOffers);
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

    // Apply bin packing to adjust group sizes
    vector<Group> final_groups;
    for (const auto& cluster : clusters) {
        if (cluster.flexOfferIDs.size() > maxGroupSize) {
            auto bins = binPackGroup(cluster, maxGroupSize, group_hash, group_hash.changeList);
            final_groups.insert(final_groups.end(), bins.begin(), bins.end());
        } else {
            final_groups.push_back(cluster);
        }
    }

    return final_groups;
}



// Optimize group to meet thresholds or size constraints
void optimizeGroup(
    int group_id, 
    Grid& grid, 
    GroupHash& group_hash, 
    const vector<int>& thresholds, 
    ChangesList& change_list, 
    const unordered_map<int, Flexoffer>& flexOffers
) {
    auto it = group_hash.groups.find(group_id);
    if (it == group_hash.groups.end()) return; // Group not found

    Group group_copy = it->second; // Copy for processing
    auto mbr = calculateMBR(group_copy.flexOfferIDs, grid.featureExtractors, flexOffers);
    if (doesMBRExceedThreshold(mbr, thresholds) || group_copy.flexOfferIDs.size() > maxGroupSize) {
        // Register removal of the group
        change_list.registerChange(group_id, '-', vector<int>(group_copy.flexOfferIDs.begin(), group_copy.flexOfferIDs.end()));
        group_hash.removeGroup(group_id);

        auto new_clusters = clusterHierarch(group_copy.cells, grid, group_hash, thresholds, flexOffers);
        for (Group& new_group : new_clusters) {
            group_hash.groups[new_group.id] = new_group;
            for (const auto& cell : new_group.cells) {
                group_hash.cellToGroupMap[cell] = new_group.id;
            }
            // Register addition of the new group
            change_list.registerChange(new_group.id, '+', vector<int>(new_group.flexOfferIDs.begin(), new_group.flexOfferIDs.end()));
        }
    }
}

*/

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
