#include <vector>
#include "ChangesList.h"

using namespace std;


void ChangesList::registerChange(int groupID, char type, const vector<int>& flexOfferIDs) {
    changes.emplace_back(groupID, type, flexOfferIDs);
}

void ChangesList::clear() {
    changes.clear();
}
