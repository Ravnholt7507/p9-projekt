#ifndef INC_AGGREGATION_H
#define INC_AGGREGATION_H

#include <vector>
#include <map>
#include "flexoffer.h"
#include "group.h"
#include "aggregation.h"
#include "alignments.h"

#include <vector>
#include <string>
#include <variant>
#include "flexoffer.h"
#include "parsing.h"
#include "group.h"
#include "DFO.h"
#include "tec.h"

using namespace std;


enum class AggMode {BATCH, INCREMENTAL};
enum class AggType {INSERT, REMOVE};

struct FOdelta {
    Flexoffer fo; 
    AggType type;
};

struct TecDelta {
    Tec_flexoffer fo;
    AggType type;
};

class IncAggregator{
public:
    IncAggregator();
    void init(int est_threshold, int lst_threshold, int max_group_size, Alignments align, AggMode mode, const vector<double> &spotPrices = {});
    void processDelta(const FOdelta &delta);
    vector<AggregatedFlexOffer> finalize();
    void setMode(AggMode newMode);

private:
    int est_threshold;
    int lst_threshold;
    int max_group_size;
    Alignments align;
    AggMode mode;
    vector<double> spotPrices;

    vector<Flexoffer> batchOffers;
    vector<Fo_Group> IncrGroups;

    struct MBR {
        int minEst;
        int maxEst;
        int minLst;
        int maxLst;
    };

    map<int, MBR> mbrs;

    int nextGroupId;

    void IncInsert(const Flexoffer &Fo);
    void IncRemove(const Flexoffer &Fo);

    void updateMBR(const Fo_Group &grp, MBR &mbr);
    bool canMerge(const MBR &mbr, const Flexoffer &FO);
};




class IncAggregatorTec {
public:
    IncAggregatorTec();
    void init(int est_threshold, int lst_threshold, int max_group_size, Alignments align, AggMode mode, const vector<double> &spotPrices = {});
    void setMode(AggMode M);
    void processDelta(const TecDelta &delta);
    std::vector<AggregatedFlexOffer> finalize();

private:
    int est_threshold;
    int lst_threshold;
    int max_group_size;
    Alignments align;
    AggMode mode;
    vector<double> spotPrices;

    std::vector<Tec_flexoffer> batchOffers;
    std::vector<Tec_Group> incrGroups;

    struct MBR {
        int minEst, maxEst;
        int minLst, maxLst;
    };

    std::map<int, MBR> mbrs;
    int nextGroupId;

private:
    void doIncInsert(const Tec_flexoffer &fo);
    void updateMBR(const Tec_Group &g, MBR &mb);
    bool canMerge(const MBR &mb, const Tec_flexoffer &fo);
};

#endif