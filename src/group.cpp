#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>

#include "../include/flexoffer.h"
#include "../include/group.h"
#include "../include/grid.h"

//Group Constructor
Group::Group(int value) {id = value;};

//Group Getters
int Group::get_id() const {return id;};
set<int> Group::get_flexOfferIDs() const {return flexOfferIDs;};
set<Cell> Group::get_cells() const {return cells;};

//Group Setters
void Group::set_id(int value) {id = value;};
void Group::set_flexOfferIDs(set<int> value) {flexOfferIDs = value;};
void Group::set_cells(set<Cell> value) {cells = value;};

//Group Utils
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

vector<Flexoffer> getFlexOffersById(set<int>& ids, vector<Flexoffer>& flexOffers) {
    vector<Flexoffer> result;

    for (int id : ids) {
        bool found = false;
        for (const auto& flexOffer : flexOffers) {
            if (flexOffer.get_offer_id() == id) {
                result.push_back(flexOffer);
                found = true;
                break; // Exit the inner loop once the flexOffer is found
            }
        }
        if (!found) {
            throw runtime_error("Flexoffer ID not found: " + to_string(id));
        }
    }

    return result;
}


//Constructor
GroupHash::GroupHash(Grid& gridRef) : grid(gridRef) {};

//GroupHash Getters
unordered_map<int, Group> GroupHash::get_groups() {return groups;};
unordered_map<Cell, int> GroupHash::get_cellToGroupMap() {return cellToGroupMap;};
Grid& GroupHash::get_grid() {return grid;};
int GroupHash::get_currentGroupID() {return currentGroupID;};

//GroupHash Setters
void GroupHash::set_groups(unordered_map<int, Group> value) {groups = value;};
void GroupHash::set_cellToGroupMap(unordered_map<Cell, int> value) {cellToGroupMap = value;};
void GroupHash::set_grid(Grid& value) {grid = value;};
void GroupHash::set_currentGroupID(int value) {currentGroupID = value;};

int GroupHash::generateUniqueGroupID() {
    return currentGroupID++;
}

void GroupHash::addFlexOffer(const Flexoffer& f) {
    Cell cell = grid.mapFlexOfferToCell(f);
    int groupID = cellToGroupMap.count(cell) ? cellToGroupMap[cell] : generateUniqueGroupID();
    groups[groupID].set_id(groupID);
    groups[groupID].addFlexOffer(f.get_offer_id(), cell);
    cellToGroupMap[cell] = groupID;
    grid.addFlexOffer(f);

    // Register change
   // changeList.registerChange(groupID, '+', {f.offer_id});
}

void GroupHash::removeFlexOffer(const Flexoffer& f) {
    Cell cell = grid.mapFlexOfferToCell(f);
    if (cellToGroupMap.count(cell)) {
        int groupID = cellToGroupMap[cell];
        groups[groupID].removeFlexOffer(f.get_offer_id());
        grid.removeFlexOffer(f);

        // Register change
        // changeList.registerChange(groupID, '-', {f.offer_id});

        if (groups[groupID].get_flexOfferIDs().empty()) {
            for (const auto& c : groups[groupID].get_cells()) {
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
        for (const auto& cell : groups[groupID].get_cells()) {
            cellToGroupMap.erase(cell);
        }
        groups.erase(groupID);
    }
}


void GroupHash::prettyprint() {
    for (const auto& cell : cellToGroupMap) {
        cout << "cell " << cell.first.indices[0] << "," << cell.first.indices[1] << " ";
        cout << "belongs to group: " << cell.second << '\n';
    }
}
