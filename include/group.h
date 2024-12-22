#ifndef GROUP_H
#define GROUP_H

#include <vector>
#include "flexoffer.h"
#include "aggregation.h"

class Group {
private:
    int id;
    std::vector<Flexoffer> flexoffers;

public:
    Group(int group_id);

    void addFlexOffer(const Flexoffer &fo);
    const vector<Flexoffer>& getFlexOffers() const;

    AggregatedFlexOffer createAggregatedOffer() const;

    void printAggregatedOffer() const;

    int getGroupId() const;
};

#endif // GROUP_H
