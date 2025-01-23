#ifndef GROUP_H
#define GROUP_H

#include <vector>
#include "flexoffer.h"
#include "aggregation.h"

using namespace std;
class Fo_Group {
private:
    int id;
    std::vector<Flexoffer> flexoffers;

public:
    Fo_Group(int group_id);

    void addFlexOffer(const Flexoffer &fo);
    const vector<Flexoffer>& getFlexOffers() const;

    AggregatedFlexOffer createAggregatedOffer(Alignments)const;
    AggregatedFlexOffer createAggregatedOffer(Alignments, const vector<double>&) const;

    void printAggregatedOffer(Alignments) const;
    void printAggregatedOffer(Alignments, const vector<double>&) const;

    int getGroupId() const;
};







class Tec_Group {
private:
    int id;
    std::vector<Tec_flexoffer> flexoffers;

public:
    Tec_Group(int group_id);

    void addFlexOffer(const Tec_flexoffer &fo);
    const vector<Tec_flexoffer>& getFlexOffers() const;

    AggregatedFlexOffer createAggregatedOffer(Alignments) const;
    AggregatedFlexOffer createAggregatedOffer(Alignments, const vector<double>&) const;

    void printAggregatedOffer(Alignments, const vector<double>&) const;
    void printAggregatedOffer(Alignments) const;

    int getGroupId() const;
};
#endif // GROUP_H
