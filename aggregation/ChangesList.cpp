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
class ChangeList {
public:
    vector<Change> changes;

    void registerChange(int groupID, char type, const vector<int>& flexOfferIDs) {
        changes.emplace_back(groupID, type, flexOfferIDs);
    }

    void clear() {
        changes.clear();
    }

    const vector<Change>& getChanges() const {
        return changes;
    }
};
