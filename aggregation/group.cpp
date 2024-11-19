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

// Group class
class Group {
public:
    int id;
    set<int> flexOfferIDs;
    set<Cell> cells;

    Group(int id = -1) : id(id) {}

    void addFlexOffer(int flexOfferID, const Cell& cell) {
        flexOfferIDs.insert(flexOfferID);
        cells.insert(cell);
    }

    void removeFlexOffer(int flexOfferID) {
        flexOfferIDs.erase(flexOfferID);
    }

    vector<int> getFlexOfferIDs() const {
        return vector<int>(flexOfferIDs.begin(), flexOfferIDs.end());
    }
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

    int generateUniqueGroupID() {
        return currentGroupID++;
    }

    void addFlexOffer(const Flexoffer& f) {
        Cell cell = grid.mapFlexOfferToCell(f);
        int groupID = cellToGroupMap.count(cell) ? cellToGroupMap[cell] : generateUniqueGroupID();
        groups[groupID].id = groupID;
        groups[groupID].addFlexOffer(f.offer_id, cell);
        cellToGroupMap[cell] = groupID;
        grid.addFlexOffer(f);

        // Register change
        changeList.registerChange(groupID, '+', {f.offer_id});
    }

    void removeFlexOffer(const Flexoffer& f) {
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

    Group& getGroup(int groupID) {
        return groups.at(groupID);
    }

    int getGroupIDForCell(const Cell& cell) const {
        return cellToGroupMap.at(cell);
    }

    void removeGroup(int groupID) {
        if (groups.find(groupID) != groups.end()) {
            for (const auto& cell : groups[groupID].cells) {
                cellToGroupMap.erase(cell);
            }
            groups.erase(groupID);
        }
    }
};
