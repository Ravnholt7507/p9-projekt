#include "../include/helperfunctions.h"
#include "../include/solver.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <vector>
#include <limits>

bool exceedsThreshold(int minE, int maxE, int minL, int maxL,
                      int est_threshold, int lst_threshold) {
    return ((maxE - minE) > est_threshold) ||
           ((maxL - minL) > lst_threshold);
}

// A 2D grid coordinate for earliest start (x) & latest start (y)
struct CellCoord {
    int x;
    int y;

    // For map or set ordering:
    bool operator<(const CellCoord &o) const {
        if (x < o.x) return true;
        if (x > o.x) return false;
        return (y < o.y);
    }
};

static const int NEIGH_OFFSETS[8][2] = {
    {1,0}, {-1,0}, {0,1}, {0,-1},
    {1,1}, {-1,1}, {1,-1}, {-1,-1}
};


// Each grid cell for normal FOs
struct FoCell {
    vector<Flexoffer> fos;
    int minEst = 999999;
    int maxEst = -1;
    int minLst = 999999;
    int maxLst = -1;
};

// Update bounding box for a FoCell
static void updateFoCellMBR(FoCell &cell)
{
    cell.minEst = 999999;
    cell.maxEst = -1;
    cell.minLst = 999999;
    cell.maxLst = -1;

    for (auto &f : cell.fos) {
        int e = f.get_est_hour();
        int l = f.get_lst_hour();
        if (e < cell.minEst) cell.minEst = e;
        if (e > cell.maxEst) cell.maxEst = e;
        if (l < cell.minLst) cell.minLst = l;
        if (l > cell.maxLst) cell.maxLst = l;
    }
}

// Convert a single Flexoffer to its grid cell coordinate
static CellCoord getCellCoordFo(const Flexoffer &fo, int deltaEst, int deltaLst)
{
    int e = fo.get_est_hour();
    int l = fo.get_lst_hour();
    return CellCoord{ e / deltaEst, l / deltaLst };
}

// BFS for normal FOs, returns one Fo_Group
static Fo_Group bfsFoGroup(
    const CellCoord &start,
    int group_id,
    map<CellCoord, FoCell> &grid,
    int est_threshold,
    int lst_threshold,
    set<CellCoord> &visited
)
{
    Fo_Group group(group_id);
    int gMinE = 999999, gMaxE = -1;
    int gMinL = 999999, gMaxL = -1;

    queue<CellCoord> Q;
    Q.push(start);
    visited.insert(start);

    auto &startCell = grid[start];
    gMinE = min(gMinE, startCell.minEst);
    gMaxE = max(gMaxE, startCell.maxEst);
    gMinL = min(gMinL, startCell.minLst);
    gMaxL = max(gMaxL, startCell.maxLst);

    while (!Q.empty()) {
        auto cur = Q.front(); 
        Q.pop();

        auto &cell = grid[cur];
        for (auto &fo : cell.fos) {
            group.addFlexOffer(fo);
        }

        for (int i=0; i<8; i++){
            CellCoord neigh {
                cur.x + NEIGH_OFFSETS[i][0],
                cur.y + NEIGH_OFFSETS[i][1]
            };
            auto it = grid.find(neigh);
            if (it == grid.end()) continue;
            if (visited.count(neigh)) continue;

            auto &nc = it->second;
            int propMinE = min(gMinE, nc.minEst);
            int propMaxE = max(gMaxE, nc.maxEst);
            int propMinL = min(gMinL, nc.minLst);
            int propMaxL = max(gMaxL, nc.maxLst);

            if (!exceedsThreshold(propMinE, propMaxE,
                                  propMinL, propMaxL,
                                  est_threshold, lst_threshold)) {
                visited.insert(neigh);
                Q.push(neigh);

                gMinE = propMinE; 
                gMaxE = propMaxE;
                gMinL = propMinL; 
                gMaxL = propMaxL;
            }
        }
    }
    return group;
}

struct TecFoCell {
    vector<Tec_flexoffer> fos;
    int minEst = 999999, maxEst = -1;
    int minLst = 999999, maxLst = -1;
};

static void updateTecFoCellMBR(TecFoCell &cell)
{
    cell.minEst = 999999;
    cell.maxEst = -1;
    cell.minLst = 999999;
    cell.maxLst = -1;

    for (auto &tf : cell.fos){
        int e = tf.get_est_hour();
        int l = tf.get_lst_hour();
        if (e < cell.minEst) cell.minEst = e;
        if (e > cell.maxEst) cell.maxEst = e;
        if (l < cell.minLst) cell.minLst = l;
        if (l > cell.maxLst) cell.maxLst = l;
    }
}

static CellCoord getCellCoordTec(const Tec_flexoffer &tf, int deltaEst, int deltaLst)
{
    int e = tf.get_est_hour();
    int l = tf.get_lst_hour();
    return CellCoord{ e / deltaEst, l / deltaLst };
}

static Tec_Group bfsTecGroup(
    const CellCoord &start,
    int group_id,
    map<CellCoord, TecFoCell> &grid,
    int est_threshold,
    int lst_threshold,
    set<CellCoord> &visited
)
{
    Tec_Group grp(group_id);
    int gMinE=999999, gMaxE=-1, gMinL=999999, gMaxL=-1;

    queue<CellCoord> Q;
    Q.push(start);
    visited.insert(start);

    auto &startCell = grid[start];
    gMinE = min(gMinE, startCell.minEst);
    gMaxE = max(gMaxE, startCell.maxEst);
    gMinL = min(gMinL, startCell.minLst);
    gMaxL = max(gMaxL, startCell.maxLst);

    while (!Q.empty()) {
        auto cur = Q.front(); 
        Q.pop();

        auto &thisCell = grid[cur];
        for (auto &f : thisCell.fos) {
            grp.addFlexOffer(f);
        }

        for (int i=0; i<8; i++){
            CellCoord neigh {
                cur.x + NEIGH_OFFSETS[i][0],
                cur.y + NEIGH_OFFSETS[i][1]
            };
            auto it = grid.find(neigh);
            if (it==grid.end()) continue;
            if (visited.count(neigh)) continue;

            auto &nc = it->second;
            int propMinE = min(gMinE, nc.minEst);
            int propMaxE = max(gMaxE, nc.maxEst);
            int propMinL = min(gMinL, nc.minLst);
            int propMaxL = max(gMaxL, nc.maxLst);

            if (!exceedsThreshold(propMinE, propMaxE, propMinL, propMaxL,
                                  est_threshold, lst_threshold)) {
                visited.insert(neigh);
                Q.push(neigh);
                gMinE=propMinE; gMaxE=propMaxE;
                gMinL=propMinL; gMaxL=propMaxL;
            }
        }
    }

    return grp;
}

// Build final Fo_Groups from the entire set of normal Flexoffers
static vector<Fo_Group> buildFoGroupsFromFlexOffers(
    const vector<Flexoffer> &allFos,
    int est_threshold,
    int lst_threshold,
    int /*max_group_size*/,
    int startGroupId
)
{
    // If no flexOffers, return empty
    vector<Fo_Group> result;
    if (allFos.empty()) return result;

    // 1) build a map<cellCoord, FoCell>
    int deltaEst = est_threshold + 1;
    int deltaLst = lst_threshold + 1;

    map<CellCoord, FoCell> grid;
    for (auto &fo : allFos) {
        CellCoord c = getCellCoordFo(fo, deltaEst, deltaLst);
        grid[c].fos.push_back(fo);
    }
    // 2) bounding box each cell
    for (auto &kv : grid) {
        updateFoCellMBR(kv.second);
    }
    // 3) BFS across cells => final groups
    set<CellCoord> visited;
    int gid = startGroupId;

    for (auto &kv : grid) {
        auto coord = kv.first;
        if (visited.count(coord)) continue;
        Fo_Group newG = bfsFoGroup(coord, gid++, grid,
                                   est_threshold, lst_threshold,
                                   visited);
        result.push_back(newG);
    }
    return result;
}

// Build final Tec_Groups from all Tec_flexoffers
static vector<Tec_Group> buildTecGroupsFromFlexOffers(
    const vector<Tec_flexoffer> &allTecs,
    int est_threshold,
    int lst_threshold,
    int /*max_group_size*/,
    int startGroupId
)
{
    vector<Tec_Group> result;
    if (allTecs.empty()) return result;

    int deltaEst = est_threshold + 1;
    int deltaLst = lst_threshold + 1;
    map<CellCoord, TecFoCell> grid;

    for (auto &tf : allTecs) {
        CellCoord c = getCellCoordTec(tf, deltaEst, deltaLst);
        grid[c].fos.push_back(tf);
    }
    for (auto &kv : grid) {
        updateTecFoCellMBR(kv.second);
    }

    set<CellCoord> visited;
    int gid = startGroupId;

    for (auto &kv : grid) {
        auto coord = kv.first;
        if (visited.count(coord)) continue;
        Tec_Group newG = bfsTecGroup(coord, gid++, grid,
                                     est_threshold, lst_threshold,
                                     visited);
        result.push_back(newG);
    }
    return result;
}

vector<AggregatedFlexOffer> nToMAggregation(
    vector<Flexoffer> &allFlexoffers, 
    int est_threshold, 
    int lst_threshold, 
    int max_group_size,
    Alignments align, 
    int startFo_GroupId
){
    vector<Fo_Group> groups = buildFoGroupsFromFlexOffers(
        allFlexoffers, est_threshold, lst_threshold,
        max_group_size, startFo_GroupId
    );

    vector<AggregatedFlexOffer> finalAggs;
    finalAggs.reserve(groups.size());
    for (auto &g : groups) {
        finalAggs.push_back(g.createAggregatedOffer(align));
    }
    return finalAggs;
}

vector<AggregatedFlexOffer> nToMAggregation(
    vector<Flexoffer> &allFlexoffers, 
    int est_threshold, 
    int lst_threshold, 
    int max_group_size,
    Alignments align, 
    const vector<double> &spotPrices,
    int startFo_GroupId
){
    vector<Fo_Group> groups = buildFoGroupsFromFlexOffers(
        allFlexoffers, est_threshold, lst_threshold,
        max_group_size, startFo_GroupId
    );

    vector<AggregatedFlexOffer> finalAggs;
    finalAggs.reserve(groups.size());
    for (auto &g : groups) {
        finalAggs.push_back(g.createAggregatedOffer(align, spotPrices));
    }
    return finalAggs;
}

// Same approach for Tec_flexoffer:
vector<AggregatedFlexOffer> nToMAggregation(
    vector<Tec_flexoffer> &allFlexoffers,
    int est_threshold,
    int lst_threshold,
    int max_group_size,
    Alignments align,
    int startFo_GroupId
){
    vector<Tec_Group> groups = buildTecGroupsFromFlexOffers(
        allFlexoffers, est_threshold, lst_threshold,
        max_group_size, startFo_GroupId
    );

    vector<AggregatedFlexOffer> finalAggs;
    finalAggs.reserve(groups.size());
    for (auto &g : groups) {
        finalAggs.push_back(g.createAggregatedOffer(align));
    }
    return finalAggs;
}

vector<AggregatedFlexOffer> nToMAggregation(
    vector<Tec_flexoffer> &allFlexoffers,
    int est_threshold,
    int lst_threshold,
    int max_group_size,
    Alignments align,
    const vector<double> &spotPrices,
    int startFo_GroupId
){
    vector<Tec_Group> groups = buildTecGroupsFromFlexOffers(
        allFlexoffers, est_threshold, lst_threshold,
        max_group_size, startFo_GroupId
    );

    vector<AggregatedFlexOffer> finalAggs;
    finalAggs.reserve(groups.size());
    for (auto &g : groups) {
        finalAggs.push_back(g.createAggregatedOffer(align, spotPrices));
    }
    return finalAggs;
}