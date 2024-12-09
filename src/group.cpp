#include "../include/group.h"

Group::Group(int group_id) : id(group_id), aggregatedOffer(group_id, flexoffers) {
    // Initially, aggregatedOffer is constructed with an empty vector of flexoffers
}

void Group::addFlexOffer(const Flexoffer& fo) {
    flexoffers.push_back(fo);
    // Update the aggregated flex offer
    updateAggregatedOffer();
}

const std::vector<Flexoffer>& Group::getFlexOffers() const {
    return flexoffers;
}

const AggregatedFlexOffer& Group::getAggregatedOffer() const {
    return aggregatedOffer;
}

void Group::updateAggregatedOffer() {
    // Reconstruct the aggregatedOffer with the updated flexoffers vector
    aggregatedOffer = AggregatedFlexOffer(id, flexoffers);
}

void Group::printAggregatedOffer() {
    aggregatedOffer.pretty_print();
}

int Group::getGroupId() const {
    return id;
}
