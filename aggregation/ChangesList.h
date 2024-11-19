#ifndef CHANGESLIST_H
#define CHANGESLIST_H

#include "vector"


using namespace std;

// Change struct
struct Change {
    int groupID;
    char type; // '+' for addition, '-' for removal
    vector<int> flexOfferIDs;

    Change(int groupID, char type, const vector<int>& flexOfferIDs)
        : groupID(groupID), type(type), flexOfferIDs(flexOfferIDs) {}
};

// ChangeList class
class ChangesList {
public:
    vector<Change> changes;

    void registerChange(int groupID, char type, const vector<int>& flexOfferIDs);
    void clear();
};


#endif