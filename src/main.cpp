#include <vector>
#include <unordered_map>
#include <iostream>

#include "../include/flexoffer.h"
#include "../include/generator.h"
#include "../include/group.h"
#include "../include/helperfunctions.h"

using namespace std;

int main() {
    int numOffers = 20;
    vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

    int est_threshold = 2;
    int lst_threshold = 2;

    int nextGroupId = 1;

    // Create one group per Flexoffer
    std::vector<Group> groups;
    int groupId = 1;
    for (auto& fo : flexOffers) {
        Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    // Perform bottom-up hierarchical clustering
    clusterGroup(groups, est_threshold, lst_threshold);

    // After clustering, we have a set of groups that do not exceed thresholds
    // Print their aggregated offers
    for (auto& g : groups) {
        g.printAggregatedOffer();
    }

    return 0;
}
