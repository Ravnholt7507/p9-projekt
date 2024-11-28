#include <algorithm>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <set>
#include <functional>

#include "../include/grid.h"
#include "../include/flexoffer.h"
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

    int est = localtime(&f.earliest_start_time)->tm_hour;
    int lst = localtime(&f.latest_start_time)->tm_hour;

    cell.indices.push_back(est);
    cell.indices.push_back(lst);
    return cell;
}

void Grid::prettyprint(){
    for (auto cell : cellmap) {
        cout << "Cell with dimensions: " << "\n";
        cout << "est: " << cell.first.indices[0] << "\n" << "lst: " << cell.first.indices[1] << '\n';
        cout << "has these flexOffer IDs: " << "\n";
        for (auto FO_ID : cell.second) {
            cout << FO_ID << "\n";
        }
    }
}