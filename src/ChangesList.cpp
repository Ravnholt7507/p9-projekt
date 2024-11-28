#include <vector>

#include "../include/ChangesList.h"

using namespace std;

//Constructor
void ChangesList::registerChange(int groupID, char type, const vector<int>& flexOfferIDs) {
    changes.emplace_back(groupID, type, flexOfferIDs);
}

//Getters
vector<Change> ChangesList::get_changes() const {return changes;};

//Setters
void ChangesList::set_changes(vector<Change> value) {changes = value;};


//Utils
void ChangesList::clear() {
    changes.clear();
}
