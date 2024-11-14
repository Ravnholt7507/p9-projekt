#ifndef GRID_H
#define GRID_H

#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include "../flexoffers/flexoffer.h"
#include "../aggregation/aggregation.h"

struct Cell {
    std::vector<int> indices;

    bool operator==(const Cell& other) const {
        return indices == other.indices;
    }

    bool operator<(const Cell& other) const {
        return indices < other.indices;
    }
};

// Specialize std::hash for Cell
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

// Grid class definition
class Grid {
public:
    Grid(const std::vector<std::function<int(const Flexoffer&)>>& extractors,
         const std::vector<int>& intervals)
        : featureExtractors(extractors), intervals(intervals) {}

    void addFlexOffer(const Flexoffer& f) {
        cellmap[mapFlexOfferToCell(f)].insert(f.offer_id);
    }

    void removeFlexOffer(const Flexoffer& f) {
        Cell cell = mapFlexOfferToCell(f);
        if (cellmap[cell].erase(f.offer_id) && cellmap[cell].empty()) {
            cellmap.erase(cell);
        }
    }

    const std::set<int>& getFlexOffersInCell(const Cell& cell) const {
        return cellmap.at(cell);
    }

    bool hasCell(const Cell& cell) const {
        return cellmap.count(cell) > 0;
    }

private:
    Cell mapFlexOfferToCell(const Flexoffer& f) const {
        Cell cell;
        for (size_t i = 0; i < featureExtractors.size(); ++i) {
            int value = featureExtractors[i](f) / intervals[i];
            cell.indices.push_back(value);
        }
        return cell;
    }

    std::unordered_map<Cell, std::set<int>> cellmap;
    std::vector<int> intervals;
    std::vector<std::function<int(const Flexoffer&)>> featureExtractors;
};

#endif // GRID_H