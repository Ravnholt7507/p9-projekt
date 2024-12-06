#ifndef GROUP_H
#define GROUP_H

#include <vector>
#include "flexoffer.h"
#include "aggregation.h"

class Group {
private:
    int id;
    std::vector<Flexoffer> flexoffers;
    AggregatedFlexOffer aggregatedOffer;

    // Method to update the aggregated flex offer
    void updateAggregatedOffer();

public:
    Group(int group_id);

    void addFlexOffer(const Flexoffer& fo);

    const std::vector<Flexoffer>& getFlexOffers() const;

    const AggregatedFlexOffer& getAggregatedOffer() const;

    void printAggregatedOffer();

    int getGroupId() const;
};

#endif // GROUP_H
