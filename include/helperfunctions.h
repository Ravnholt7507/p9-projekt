#ifndef HELPER_FUNCTIONS
#define HELPER_FUNCTIONS
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <climits>
#include <algorithm>
#include <functional>

//#include "ChangesList.h"
#include "flexoffer.h"
#include "group.h"
#include "grid.h"

using namespace std;

pair<vector<int>, vector<int>> calculateMBR(vector<Flexoffer>& flexOffers);

bool doesMBRExceedThreshold(const pair<vector<int>, vector<int>>& mbr, const vector<int>& thresholds);

vector<Group> binPackGroup(const Group& group, int max_size, GroupHash& group_hash);

vector<Group> clusterHierarch(const set<Cell>& cells, GroupHash& group_hash, vector<int> thresholds, vector<Flexoffer> flexOffers);

void optimizeGroup(int group_id, GroupHash &gh, vector<int> thresholds, vector<Flexoffer> &flexOffers);

void deltaProcess(const Flexoffer& offer, char action, GroupHash& group_hash);

#endif
