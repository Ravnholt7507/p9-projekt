#ifndef GROUP_H
#define GROUP_H
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <climits>
#include <algorithm>
#include <functional>

#include "grid.h"
#include "ChangesList.h"
#include "flexoffer.h"
#include "aggregation.h"

// Group class
class Group {
    private: 
        int id;
        set<int> flexOfferIDs;
        set<Cell> cells;

    public:
        //Constructor
        Group(int value = -1);

        //Getters
        int get_id() const;
        set<int> get_flexOfferIDs() const;
        set<Cell> get_cells() const;

        //Setters
        void set_id(int);
        void set_flexOfferIDs(set<int>);
        void set_cells(set<Cell>);

        //Utils
        void addFlexOffer(int flexOfferID, const Cell& cell);
        void removeFlexOffer(int flexOfferID);
        vector<int> getFlexOfferIDs();
};



// GroupHash class
class GroupHash {
    private:
        unordered_map<int, Group> groups;
        unordered_map<Cell, int> cellToGroupMap;
        Grid& grid;
        int currentGroupID = 0;
    public:
        //Constructor
        GroupHash(Grid& gridRef); 
        
        //Getters
        unordered_map<int, Group> get_groups();
        unordered_map<Cell, int> get_cellToGroupMap();
        Grid& get_grid();
        int get_currentGroupID();

        //Setters
        void set_groups(unordered_map<int, Group>);
        void set_cellToGroupMap(unordered_map<Cell, int>);
        void set_grid(Grid&);
        void set_currentGroupID(int);

        //Utils
        int generateUniqueGroupID();
        void addFlexOffer(const Flexoffer& f);
        void removeFlexOffer(const Flexoffer& f);
        Group& getGroup(int groupID);
        int getGroupIDForCell(const Cell& cell);
        void removeGroup(int groupID);
        void prettyprint();
};

vector<Flexoffer> getFlexOffersById(set<int> &ids, vector<Flexoffer> &flexOffers);

#endif
