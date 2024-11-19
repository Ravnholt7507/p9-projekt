#ifndef GRID_H
#define GRID_H

#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include "../flexoffers/flexoffer.h"

// Struct definition for Cell
struct Cell {
    std::vector<int> indices;
    bool operator==(const Cell& other) const;
    bool operator<(const Cell& other) const;
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

    void addFlexOffer(const Flexoffer& f);
    void removeFlexOffer(const Flexoffer& f);
    const std::set<int>& getFlexOffersInCell(const Cell& cell) const;
    bool hasCell(const Cell& cell) const;
    Cell mapFlexOfferToCell(const Flexoffer& f) const;

    std::unordered_map<Cell, std::set<int>> cellmap;
    std::vector<int> intervals;
    std::vector<std::function<int(const Flexoffer&)>> featureExtractors;
};

#endif // GRID_H