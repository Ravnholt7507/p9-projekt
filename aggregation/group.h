#ifndef GROUP_H
#define GROUP_H


#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <climits>
#include <algorithm>
#include <functional>
#include <grid.h>
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

#endif
