#include "../include/group.h"
#include "../include/alignments.h"

Fo_Group::Fo_Group(int group_id) : id(group_id) {
}

void Fo_Group::addFlexOffer(const Flexoffer& fo) {
    flexoffers.push_back(fo);
}

const std::vector<Flexoffer>& Fo_Group::getFlexOffers() const {
    return flexoffers;
}

AggregatedFlexOffer Fo_Group::createAggregatedOffer(Alignments align) const {
    return AggregatedFlexOffer(id, align, flexoffers);
}

AggregatedFlexOffer Fo_Group::createAggregatedOffer(Alignments align, const vector<double> &spotPrices) const {
    return AggregatedFlexOffer(id, align, flexoffers, spotPrices);
}

void Fo_Group::printAggregatedOffer(Alignments align) const {
    auto agg = createAggregatedOffer(align);
    agg.pretty_print();
}

void Fo_Group::printAggregatedOffer(Alignments align, const vector<double> &spotPrices) const {
    auto agg = createAggregatedOffer(align, spotPrices);
    agg.pretty_print();
}

int Fo_Group::getGroupId() const {
    return id;
}







Tec_Group::Tec_Group(int group_id) : id(group_id) {
}

void Tec_Group::addFlexOffer(const Tec_flexoffer& fo) {
    flexoffers.push_back(fo);
}

const std::vector<Tec_flexoffer>& Tec_Group::getFlexOffers() const {
    return flexoffers;
}

AggregatedFlexOffer Tec_Group::createAggregatedOffer(Alignments align) const {
    return AggregatedFlexOffer(id, align, flexoffers);
}

AggregatedFlexOffer Tec_Group::createAggregatedOffer(Alignments align, const vector<double> &spotPrices) const {
    return AggregatedFlexOffer(id, align, flexoffers, spotPrices);
}

void Tec_Group::printAggregatedOffer(Alignments align) const {
    auto agg = createAggregatedOffer(align);
    agg.pretty_print();
}

void Tec_Group::printAggregatedOffer(Alignments align, const vector<double> &spotPrices) const {
    auto agg = createAggregatedOffer(align, spotPrices);
    agg.pretty_print();
}

int Tec_Group::getGroupId() const {
    return id;
}
