#include "iostream"
#include "vector"
#include "unordered_map"
#include "set"
#include "climits"
#include "algorithm"
#include "functional"
#include "../flexoffers/flexoffer.h"
#include "grid.h"
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