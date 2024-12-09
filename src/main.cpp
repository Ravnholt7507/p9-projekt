#include "../include/flexoffer.h"
#include "../include/generator.h"
#include "../include/group.h"
#include "../include/helperfunctions.h"

using namespace std;

int main() {
    int numOffers = 2;
    vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

    // Create one group per Flexoffer
    std::vector<Group> groups;
    int groupId = 1;
    for (const auto& fo : flexOffers) {
        Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    int est_threshold = 2; // 1 hour
    int lst_threshold = 2; // 1 hour
    int max_group_size = 3; // For example, no more than 3 Flexoffers per group

    // Perform bottom-up hierarchical clustering with max_group_size constraint
    clusterGroup(groups, est_threshold, lst_threshold, max_group_size);

    // Print final groups
    for (auto& g : groups) {
        g.printAggregatedOffer();
    }
    return 0;
}
