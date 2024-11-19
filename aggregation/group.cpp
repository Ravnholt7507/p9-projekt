#include "iostream"
#include "vector"
#include "unordered_map"
#include "set"
#include "climits"
#include "algorithm"
#include "ChangesList.h"
#include "functional"
#include "../flexoffers/flexoffer.h"
#include "group.h"
#include "grid.h"


void Group::addFlexOffer(int flexOfferID, const Cell& cell) {
    flexOfferIDs.insert(flexOfferID);
    cells.insert(cell);
}

void Group::removeFlexOffer(int flexOfferID) {
    flexOfferIDs.erase(flexOfferID);
}

vector<int> Group::getFlexOfferIDs() {
    return vector<int>(flexOfferIDs.begin(), flexOfferIDs.end());
}


int GroupHash::generateUniqueGroupID() {
    return currentGroupID++;
}

void GroupHash::addFlexOffer(const Flexoffer& f) {
    Cell cell = grid.mapFlexOfferToCell(f);
    int groupID = cellToGroupMap.count(cell) ? cellToGroupMap[cell] : generateUniqueGroupID();
    groups[groupID].id = groupID;
    groups[groupID].addFlexOffer(f.offer_id, cell);
    cellToGroupMap[cell] = groupID;
    grid.addFlexOffer(f);

    // Register change
    changeList.registerChange(groupID, '+', {f.offer_id});
}

void GroupHash::removeFlexOffer(const Flexoffer& f) {
    Cell cell = grid.mapFlexOfferToCell(f);
    if (cellToGroupMap.count(cell)) {
        int groupID = cellToGroupMap[cell];
        groups[groupID].removeFlexOffer(f.offer_id);
        grid.removeFlexOffer(f);

        // Register change
        changeList.registerChange(groupID, '-', {f.offer_id});

        if (groups[groupID].flexOfferIDs.empty()) {
            for (const auto& c : groups[groupID].cells) {
                cellToGroupMap.erase(c);
            }
            groups.erase(groupID);
        }
    }
}

Group& GroupHash::getGroup(int groupID) {
    return groups.at(groupID);
}

int GroupHash::getGroupIDForCell(const Cell& cell) {
    return cellToGroupMap.at(cell);
}

void GroupHash::removeGroup(int groupID) {
    if (groups.find(groupID) != groups.end()) {
        for (const auto& cell : groups[groupID].cells) {
            cellToGroupMap.erase(cell);
        }
        groups.erase(groupID);
    }
}
