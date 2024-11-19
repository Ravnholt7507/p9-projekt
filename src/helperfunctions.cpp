#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <climits>
#include <algorithm>
#include <functional>

#include "../include/ChangesList.h"
#include "../include/flexoffer.h"
#include "../include/group.h"
#include "../include/grid.h"

const int maxGroupSize = 2;

// Compute Minimum Bounding Rectangle (MBR)
std::pair<std::vector<int>, std::vector<int>> calculateMBR(
    const std::set<int>& offer_ids, 
    const std::vector<std::function<int(const Flexoffer&)>>& extractors,
    const std::unordered_map<int, Flexoffer>& flexOffers
) {
    std::vector<int> mins(extractors.size(), INT_MAX);
    std::vector<int> maxs(extractors.size(), INT_MIN);

    for (int id : offer_ids) {
        const auto& offer = flexOffers.at(id); // Get the FlexOffer by ID
        for (size_t i = 0; i < extractors.size(); ++i) {
            int value = extractors[i](offer);
            mins[i] = std::min(mins[i], value);
            maxs[i] = std::max(maxs[i], value);
        }
    }

    return {mins, maxs};
}

// Check if MBR exceeds thresholds
bool doesMBRExceedThreshold(
    const std::pair<std::vector<int>, std::vector<int>>& mbr, 
    const std::vector<int>& thresholds
) {
    for (size_t i = 0; i < mbr.first.size(); ++i) {
        if (mbr.second[i] - mbr.first[i] > thresholds[i]) {
            return true;
        }
    }
    return false;
}

// Bin packing algorithm
std::vector<Group> binPackGroup(
    const Group& group, 
    int max_size, 
    GroupHash& group_hash, 
    ChangesList& change_list
) {
    std::vector<Group> bins;
    std::vector<int> offer_ids(group.flexOfferIDs.begin(), group.flexOfferIDs.end());

    size_t index = 0;
    while (index < offer_ids.size()) {
        Group bin(group_hash.generateUniqueGroupID());
        bin.cells = group.cells; // All bins share the same cells

        for (int i = 0; i < max_size && index < offer_ids.size(); ++i, ++index) {
            bin.flexOfferIDs.insert(offer_ids[index]);
        }
        bins.push_back(bin);

        // Register the addition of the new bin
        change_list.registerChange(bin.id, '+', std::vector<int>(bin.flexOfferIDs.begin(), bin.flexOfferIDs.end()));
    }
    return bins;
}

// Hierarchical clustering with MBR checks
std::vector<Group> clusterHierarch(
    const std::set<Cell>& cells, 
    Grid& grid, 
    GroupHash& group_hash, 
    const std::vector<int>& thresholds, 
    const std::unordered_map<int, Flexoffer>& flexOffers
) {
    std::vector<Group> clusters;
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
        std::vector<Group> new_clusters;

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
        clusters = std::move(new_clusters);
    }

    // Apply bin packing to adjust group sizes
    std::vector<Group> final_groups;
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
    const std::vector<int>& thresholds, 
    ChangesList& change_list, 
    const std::unordered_map<int, Flexoffer>& flexOffers
) {
    auto it = group_hash.groups.find(group_id);
    if (it == group_hash.groups.end()) return; // Group not found

    Group group_copy = it->second; // Copy for processing
    auto mbr = calculateMBR(group_copy.flexOfferIDs, grid.featureExtractors, flexOffers);
    if (doesMBRExceedThreshold(mbr, thresholds) || group_copy.flexOfferIDs.size() > maxGroupSize) {
        // Register removal of the group
        change_list.registerChange(group_id, '-', std::vector<int>(group_copy.flexOfferIDs.begin(), group_copy.flexOfferIDs.end()));
        group_hash.removeGroup(group_id);

        auto new_clusters = clusterHierarch(group_copy.cells, grid, group_hash, thresholds, flexOffers);
        for (Group& new_group : new_clusters) {
            group_hash.groups[new_group.id] = new_group;
            for (const auto& cell : new_group.cells) {
                group_hash.cellToGroupMap[cell] = new_group.id;
            }
            // Register addition of the new group
            change_list.registerChange(new_group.id, '+', std::vector<int>(new_group.flexOfferIDs.begin(), new_group.flexOfferIDs.end()));
        }
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
