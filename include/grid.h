#ifndef GRID_H
#define GRID_H
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>

#include "flexoffer.h"

using namespace std; 

// Struct definition for Cell
struct Cell {
    vector<int> indices;
    bool operator==(const Cell& other) const;
    bool operator<(const Cell& other) const;
};


// Specialize hash for Cell
namespace std {
    template <>
    struct hash<Cell> {
        size_t operator()(const Cell& cell) const {
            size_t seed = 0;
            for (int index : cell.indices) {
                seed ^= hash<int>()(index) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}

// Grid class definition
class Grid {
public:
    // Constructor
    Grid(){

    }

    void addFlexOffer(const Flexoffer& f);
    void removeFlexOffer(const Flexoffer& f);
    void prettyprint();
    const set<int>& getFlexOffersInCell(const Cell& cell) const;
    bool hasCell(const Cell& cell) const;
    Cell mapFlexOfferToCell(const Flexoffer& f) const;

    unordered_map<Cell, set<int>> cellmap;
};

#endif // GRID_H
