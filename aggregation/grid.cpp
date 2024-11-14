#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include "../flexoffers/flexoffer.h"
#include <climits>
#include <algorithm>
#include <functional>

using namespace std;


// Struct for Cell
struct Cell {
    vector<int> indices;

    bool operator==(const Cell& other) const {
        return indices == other.indices;
    }

    bool operator<(const Cell& other) const {
        return indices < other.indices;
    }
};

// Hash function for Cell (this is needed to use cells as keys)
namespace std {
    template <>
    struct hash<Cell> {
        size_t operator()(const Cell& cell) const {
            size_t seed = 0;
            for (int index : cell.indices) {
                seed ^= std::hash<int>()(index) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}


// Grid class
class Grid {
public:
    unordered_map<Cell, set<int>> cellmap;
    vector<int> intervals;
    vector<function<int(const Flexoffer&)>> featureExtractors;

    Grid(const vector<function<int(const Flexoffer&)>>& extractors, const vector<int>& intervals)
        : featureExtractors(extractors), intervals(intervals) {}

    Cell mapFlexOfferToCell(const Flexoffer& f) const {
        Cell cell;
        for (size_t i = 0; i < featureExtractors.size(); ++i) {
            int value = featureExtractors[i](f) / intervals[i];
            cell.indices.push_back(value);
        }
        return cell;
    }

    void addFlexOffer(const Flexoffer& f) {
        cellmap[mapFlexOfferToCell(f)].insert(f.offer_id);
    }

    void removeFlexOffer(const Flexoffer& f) {
        Cell cell = mapFlexOfferToCell(f);
        if (cellmap[cell].erase(f.offer_id) && cellmap[cell].empty()) {
            cellmap.erase(cell);
        }
    }

    const set<int>& getFlexOffersInCell(const Cell& cell) const {
        return cellmap.at(cell);
    }

    bool hasCell(const Cell& cell) const {
        return cellmap.count(cell);
    }
};
