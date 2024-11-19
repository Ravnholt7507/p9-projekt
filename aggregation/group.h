#ifndef GROUP_H
#define GROUP_H


#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <climits>
#include <algorithm>
#include <functional>
#include "ChangesList.h"
#include "grid.h"
#include "../flexoffers/flexoffer.h"
#include "../aggregation/aggregation.h"

// Group class
class Group {
public:
    int id;
    set<int> flexOfferIDs;
    set<Cell> cells;
    Group(int id = -1) : id(id) {}
    void addFlexOffer(int flexOfferID, const Cell& cell);
    void removeFlexOffer(int flexOfferID);
    vector<int> getFlexOfferIDs();
};



// GroupHash class
class GroupHash {
public:
    unordered_map<int, Group> groups;
    unordered_map<Cell, int> cellToGroupMap;
    Grid& grid;
    ChangeList& changeList;
    int currentGroupID = 0;

    GroupHash(Grid& gridRef, ChangeList& clRef) : grid(gridRef), changeList(clRef) {}
    int generateUniqueGroupID();
    void addFlexOffer(const Flexoffer& f);
    void removeFlexOffer(const Flexoffer& f);
    Group& getGroup(int groupID);
    int getGroupIDForCell(const Cell& cell);
    void removeGroup(int groupID);
};
#endif
