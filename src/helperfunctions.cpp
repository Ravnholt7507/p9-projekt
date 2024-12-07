#include "../include/helperfunctions.h"
#include <algorithm>
#include <cmath>
#include <limits>

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

// Compute the "distance" between two groups based on their MBR centroids
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
    return sqrt(dx * dx + dy * dy);
}

// Merge two groups into a new group
static Group mergeGroups(const Group& g1, const Group& g2, int newGroupId) {
    Group merged(newGroupId);
    // Add all Flexoffers from g1 and g2
    for (const auto& fo : g1.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    for (const auto& fo : g2.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    return merged;
}

// Bottom-up hierarchical clustering with group size constraint
void clusterGroup(vector<Group>& groups, int est_threshold, int lst_threshold, int max_group_size) {
    if (groups.size() <= 1) return;

    bool merged = true;
    int nextGroupId = 1000; // Arbitrary start for new group IDs

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
            // No pairs found
            break;
        }

        // Attempt to merge the two closest groups
        Group candidate = mergeGroups(groups[bestA], groups[bestB], nextGroupId++);
        MBR candidateMBR;
        createMBR(candidate, candidateMBR);

        // Check both MBR threshold and max_group_size constraint
        if (!exceedsThreshold(candidateMBR, est_threshold, lst_threshold) &&
            (int)candidate.getFlexOffers().size() <= max_group_size) {
            // Merge is acceptable
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
