#ifndef GROUP_H
#define GROUP_H

#include <vector>
#include "flexoffer.h"
#include "DFO.h"
#include "aggregation.h"

using namespace std;
class Fo_Group {
private:
    int id;
    vector<Flexoffer> flexoffers;

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
    vector<Tec_flexoffer> flexoffers;

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

struct Dfo_Group {
    int group_id;
    vector<DFO> dfos;
    Dfo_Group(int gid) : group_id(gid) {}
    void add(const DFO &d){ dfos.push_back(d); }
    const vector<DFO>& getDFOs() const {return dfos;}
};

#endif
