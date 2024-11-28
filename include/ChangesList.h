#ifndef CHANGESLIST_H
#define CHANGESLIST_H
#include <vector>

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
    vector<Change> changes;
public:
    //Contructor
    void registerChange(int groupID, char type, const vector<int>& flexOfferIDs);

    //Getters
    vector<Change> get_changes() const;

    //Setters
    void set_changes(vector<Change>);

    //Utils
    void clear();
};


#endif
