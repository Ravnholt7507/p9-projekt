#include "../include/group.h"


Group::Group(int group_id) : id(group_id) {
}

void Group::addFlexOffer(const Flexoffer& fo) {
    flexoffers.push_back(fo);
}

const std::vector<Flexoffer>& Group::getFlexOffers() const {
    return flexoffers;
}

AggregatedFlexOffer Group::createAggregatedOffer() const {
    return AggregatedFlexOffer(id, flexoffers);
}


void Group::printAggregatedOffer() const {
    auto agg = createAggregatedOffer();
    agg.pretty_print();
}

int Group::getGroupId() const {
    return id;
}
