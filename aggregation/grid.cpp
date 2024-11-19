#include <algorithm>
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include "grid.h"
#include "../flexoffers/flexoffer.h"
using namespace std;

// Define Cell operators
bool Cell::operator==(const Cell& other) const {
    return indices == other.indices;
}

bool Cell::operator<(const Cell& other) const {
    return indices < other.indices;
}

// Grid methods
void Grid::addFlexOffer(const Flexoffer& f) {
    Cell cell = mapFlexOfferToCell(f);
    cellmap[cell].insert(f.offer_id);
}

void Grid::removeFlexOffer(const Flexoffer& f) {
    Cell cell = mapFlexOfferToCell(f);
    if (cellmap.count(cell)) {
        cellmap[cell].erase(f.offer_id);
        if (cellmap[cell].empty()) {
            cellmap.erase(cell);
        }
    }
}

const set<int>& Grid::getFlexOffersInCell(const Cell& cell) const {
    if (!hasCell(cell)) {
        throw out_of_range("Cell not found in the grid.");
    }
    return cellmap.at(cell);
}

bool Grid::hasCell(const Cell& cell) const {
    return cellmap.find(cell) != cellmap.end();
}

Cell Grid::mapFlexOfferToCell(const Flexoffer& f) const {
    Cell cell;
    for (size_t i = 0; i < featureExtractors.size(); ++i) {
        int value = featureExtractors[i](f);
        cell.indices.push_back(value / intervals[i]);
    }
    return cell;
}