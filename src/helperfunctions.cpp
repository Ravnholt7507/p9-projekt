#include "../include/helperfunctions.h"
#include "../include/solver.h"
#include <algorithm>
#include <fstream>
#include <chrono>
#include <fstream>
#include <chrono>  
#include <iostream>
#include <map>
#include <sstream>
#include <cmath>
#include <limits>
#include <iostream>
#include <queue>
#include <set>
#include <cstdlib>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <utility>

using namespace std;

struct cellCoord {
    int x;
    int y;
    bool operator<(const cellCoord &o) const {
        if (x < o.x) return true;
        if (x > o.x) return false;
        return (y < o.y);
    }
};

struct TecFoCell {
    vector<Tec_flexoffer> tec_fos;
    int minEst = 999999, maxEst = -1;
    int minLst = 999999, maxLst = -1;
};


static const int NEIGH_OFFSETS[8][2] = {
    {1,0}, {-1,0}, {0,1}, {0,-1},
    {1,1}, {-1,1}, {1,-1}, {-1,-1}
};


void updateTecFoCellMBR(TecFoCell cell){
    cell.minEst = 999999;
    cell.maxEst = -1;
    cell.minLst = 999999;
    cell.maxLst = -1;
    for (auto &f : cell.tec_fos){
        int e = f.get_est_hour();
        int l = f.get_lst_hour();
        if (e < cell.minEst) cell.minEst = e;
        if (e > cell.maxEst) cell.maxEst = e;
        if (e < cell.minLst) cell.minLst = l;
        if (e > cell.minLst) cell.minLst = l;
    }
}


//For Fo
void createMBR(const Fo_Group& group, MBR& mbr) {
    const auto& flexoffers = group.getFlexOffers();
    if (flexoffers.empty()) return;

    mbr.min_est_hour = flexoffers.front().get_est_hour();
    mbr.max_est_hour = flexoffers.front().get_est_hour();
    mbr.min_lst_hour = flexoffers.front().get_lst_hour();
    mbr.max_lst_hour = flexoffers.front().get_lst_hour();

    for (const auto& fo : flexoffers) {
        int est_hour = fo.get_est_hour();
        int lst_hour = fo.get_lst_hour();

        if (est_hour < mbr.min_est_hour) mbr.min_est_hour = est_hour;
        if (est_hour > mbr.max_est_hour) mbr.max_est_hour = est_hour;
        if (lst_hour < mbr.min_lst_hour) mbr.min_lst_hour = lst_hour;
        if (lst_hour > mbr.max_lst_hour) mbr.max_lst_hour = lst_hour;
    }
}

//For Tec
void createMBR(const Tec_Group& group, MBR& mbr) {
    const auto& flexoffers = group.getFlexOffers();
    if (flexoffers.empty()) return;

    mbr.min_est_hour = flexoffers.front().get_est_hour();
    mbr.max_est_hour = flexoffers.front().get_est_hour();
    mbr.min_lst_hour = flexoffers.front().get_lst_hour();
    mbr.max_lst_hour = flexoffers.front().get_lst_hour();

    for (const auto& fo : flexoffers) {
        int est_hour = fo.get_est_hour();
        int lst_hour = fo.get_lst_hour();

        if (est_hour < mbr.min_est_hour) mbr.min_est_hour = est_hour;
        if (est_hour > mbr.max_est_hour) mbr.max_est_hour = est_hour;
        if (lst_hour < mbr.min_lst_hour) mbr.min_lst_hour = lst_hour;
        if (lst_hour > mbr.max_lst_hour) mbr.max_lst_hour = lst_hour;
    }
}


bool exceedsThreshold(const MBR& mbr, int est_threshold, int lst_threshold) {
    int est_range = mbr.max_est_hour - mbr.min_est_hour;
    int lst_range = mbr.max_lst_hour - mbr.min_lst_hour;
    return est_range > est_threshold || lst_range > lst_threshold;
}

bool exceedsThreshold(int minE, int maxE, int minL, int maxL, int est_threshold, int lst_threshold) {
    return ((maxE - minE) > est_threshold) || ((maxL - minL) > lst_threshold);
}

//For Fo
static double groupDistance(const Fo_Group& g1, const Fo_Group& g2) {
    MBR m1, m2;
    createMBR(g1, m1);
    createMBR(g2, m2);

    double c1_est = (m1.min_est_hour + m1.max_est_hour) / 2.0;
    double c1_lst = (m1.min_lst_hour + m1.max_lst_hour) / 2.0;
    double c2_est = (m2.min_est_hour + m2.max_est_hour) / 2.0;
    double c2_lst = (m2.min_lst_hour + m2.max_lst_hour) / 2.0;

    double dx = c2_est - c1_est;
    double dy = c2_lst - c1_lst;
    return sqrt(dx*dx + dy*dy);
}

//for Tec
static double groupDistance(const Tec_Group& g1, const Tec_Group& g2) {
    MBR m1, m2;
    createMBR(g1, m1);
    createMBR(g2, m2);

    double c1_est = (m1.min_est_hour + m1.max_est_hour) / 2.0;
    double c1_lst = (m1.min_lst_hour + m1.max_lst_hour) / 2.0;
    double c2_est = (m2.min_est_hour + m2.max_est_hour) / 2.0;
    double c2_lst = (m2.min_lst_hour + m2.max_lst_hour) / 2.0;

    double dx = c2_est - c1_est;
    double dy = c2_lst - c1_lst;
    return sqrt(dx*dx + dy*dy);
}

//For Fo
static Fo_Group mergeGroups(const Fo_Group& g1, const Fo_Group& g2, int newGroupId) {
    Fo_Group merged(newGroupId);
    for (const auto& fo : g1.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    for (const auto& fo : g2.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    return merged;
}

//For Tec
static Tec_Group mergeGroups(const Tec_Group& g1, const Tec_Group& g2, int newGroupId) {
    Tec_Group merged(newGroupId);
    for (const auto& fo : g1.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    for (const auto& fo : g2.getFlexOffers()) {
        merged.addFlexOffer(fo);
    }
    return merged;
}

//For Fo
void clusterFo_Group(vector<Fo_Group>& groups, int est_threshold, int lst_threshold, int max_group_size) {

    bool merged = true;
    int nextGroupId = 1000;


    while (merged && groups.size() > 1) {
        merged = false;
        double minDist = numeric_limits<double>::max();
        int bestA = -1, bestB = -1;

        // Find the two closest groups
        for (size_t i = 0; i < groups.size(); ++i) {
            for (size_t j = i + 1; j < groups.size(); ++j) {
                double dist = groupDistance(groups[i], groups[j]);
                if (dist < minDist) {
                    minDist = dist;
                    bestA = (int)i;
                    bestB = (int)j;
                }
            }
        }

        if (bestA == -1 || bestB == -1) {
            break;
        }

        Fo_Group candidate = mergeGroups(groups[bestA], groups[bestB], nextGroupId++);
        MBR candidateMBR;
        createMBR(candidate, candidateMBR);

        bool thresholdOK = !exceedsThreshold(candidateMBR, est_threshold, lst_threshold);
        bool sizeOK = (int)candidate.getFlexOffers().size() <= max_group_size;

        if (thresholdOK && sizeOK) {
            if (bestA > bestB) swap(bestA, bestB);
            groups.erase(groups.begin() + bestB);
            groups.erase(groups.begin() + bestA);
            groups.push_back(candidate);
            merged = true;
        } else {
            merged = false;
        }
    }

}

auto BFS(cellCoord &start, int group_id, map<cellCoord, TecFoCell> &grid, int est_th, int lst_th, set<cellCoord> &visited){
    Tec_Group grp(group_id);
    int groupMinEst = 99999, groupMaxEst = -1, groupMinLst = 99999, groupMaxLst = -1;

    queue<cellCoord> Q;
    Q.push(start);
    visited.insert(start);


    auto &sc = grid[start];
    groupMinEst = min(groupMinEst, sc.minEst);
    groupMaxEst = max(groupMaxEst, sc.maxEst);
    groupMinLst  = min(groupMinLst, sc.minLst);
    groupMaxLst = max(groupMaxLst, sc.maxLst);

    while(!Q.empty()) {
        auto cur = Q.front(); 
        Q.pop();
        auto &thisCell = grid[cur];
        // add tec_flexoffers
        for (auto &tf : thisCell.tec_fos) {
            grp.addFlexOffer(tf);
        }

        // neighbors
        for (int i=0; i<8; i++){
            cellCoord neigh { cur.x+NEIGH_OFFSETS[i][0], cur.y+NEIGH_OFFSETS[i][1] };
            auto it = grid.find(neigh);
            if (it==grid.end()) continue;
            if (visited.count(neigh)) continue;

            auto &nc = it->second;
            int propMinE = min(groupMinEst, nc.minEst);
            int propMaxE = max(groupMaxEst, nc.maxEst);
            int propMinL = min(groupMinLst, nc.minLst);
            int propMaxL = max(groupMaxLst, nc.maxLst);

            if (!exceedsThreshold(propMinE, propMaxE, propMinL, propMaxL, est_th, lst_th)) {
                visited.insert(neigh);
                Q.push(neigh);
                groupMinEst=propMinE; 
                groupMaxEst=propMaxE;
                groupMinLst=propMinL; 
                groupMaxLst=propMaxL;
            }
        }
    }

    return grp;
}


auto getCellCoord(const Tec_flexoffer &fo, int est_threshold, int lst_threshold){
    int e = fo.get_est_hour();
    int l = fo.get_lst_hour();
    int x = e / est_threshold;
    int y = l / lst_threshold;
    return cellCoord{x, y};
}


//For tec
void clusterFo_Group(vector<Tec_Group>& groups, int est_threshold, int lst_threshold, int max_group_size) {
    vector<Tec_flexoffer> allTec;

    for (auto &g : groups){
        for (auto &f : g.getFlexOffers()) {
            allTec.push_back(f);
        }
    }

    map<cellCoord, TecFoCell>grid;
    groups.clear();

    for (auto &fo : allTec){
        auto cellcoords = getCellCoord(fo, est_threshold, lst_threshold);
        auto &cell = grid[cellcoords];
        cell.tec_fos.push_back(fo);
    }

    for (auto &cell : grid) {
        updateTecFoCellMBR(cell.second);
    }

    set<cellCoord> visited;
    visited.clear();

    int nextId = 1;

    for (auto &cell : grid){
        auto coord = cell.first;
        if (visited.count(coord)) continue;
        auto newG = BFS(coord, nextId, grid, est_threshold, lst_threshold, visited);
        groups.push_back(newG);
    }
}

//For FO
vector<AggregatedFlexOffer> nToMAggregation(vector<Flexoffer> &allFlexoffers, 
                                            int est_threshold, 
                                            int lst_threshold, 
                                            int max_group_size,
                                            Alignments align, 
                                            int startFo_GroupId=1)
{
    vector<Fo_Group> groups;
    int groupId = startFo_GroupId;
    for (const auto &fo : allFlexoffers) {
        Fo_Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    clusterFo_Group(groups, est_threshold, lst_threshold, max_group_size);

    vector<AggregatedFlexOffer> finalAggregates;
    finalAggregates.reserve(groups.size());
    for (auto &g : groups) {
        finalAggregates.push_back(g.createAggregatedOffer(align));
    }

    // 7) Return the final AFOs
    return finalAggregates;
}
vector<AggregatedFlexOffer> nToMAggregation(vector<Flexoffer> &allFlexoffers, 
                                            int est_threshold, 
                                            int lst_threshold, 
                                            int max_group_size,
                                            Alignments align, 
                                            const vector<double> &spotPrices,
                                            int startFo_GroupId=1)
{
    vector<Fo_Group> groups;
    int groupId = startFo_GroupId;
    for (const auto &fo : allFlexoffers) {
        Fo_Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    clusterFo_Group(groups, est_threshold, lst_threshold, max_group_size);

    vector<AggregatedFlexOffer> finalAggregates;
    finalAggregates.reserve(groups.size());
    for (auto &g : groups) {
        finalAggregates.push_back(g.createAggregatedOffer(align, spotPrices));
    }

    return finalAggregates;
}

//For tec
vector<AggregatedFlexOffer> nToMAggregation(vector<Tec_flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align,int startFo_GroupId=1){
    vector<Tec_Group> groups;
    int groupId = startFo_GroupId;
    for (const auto &fo : allFlexoffers) {
        Tec_Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    clusterFo_Group(groups, est_threshold, lst_threshold, max_group_size);

    vector<AggregatedFlexOffer> finalAggregates;
    finalAggregates.reserve(groups.size());
    for (auto &g : groups) {
        finalAggregates.push_back(g.createAggregatedOffer(align));
    }

    return finalAggregates;
}


vector<AggregatedFlexOffer> nToMAggregation(vector<Tec_flexoffer> &allFlexoffers, int est_threshold, int lst_threshold, int max_group_size, Alignments align,const vector<double> &spotPrices,int startFo_GroupId=1){
    vector<Tec_Group> groups;
    int groupId = startFo_GroupId;
    for (const auto &fo : allFlexoffers) {
        Tec_Group g(groupId++);
        g.addFlexOffer(fo);
        groups.push_back(g);
    }

    clusterFo_Group(groups, est_threshold, lst_threshold, max_group_size);

    vector<AggregatedFlexOffer> finalAggregates;
    finalAggregates.reserve(groups.size());
    for (auto &g : groups) {
        finalAggregates.push_back(g.createAggregatedOffer(align, spotPrices));
    }

    return finalAggregates;
}



