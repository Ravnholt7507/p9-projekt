#ifndef HELPER_FUNCTIONS
#define HELPER_FUNCTIONS
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <climits>
#include <algorithm>
#include <functional>

#include "ChangesList.h"
#include "flexoffer.h"
#include "group.h"
#include "grid.h"

using namespace std;

const int maxGroupSize = 2;

pair<vector<int>, vector<int>> calculateMBR(vector<Flexoffer>& flexOffers);

bool doesMBRExceedThreshold(const pair<vector<int>, vector<int>>& mbr, const vector<int>& thresholds);

vector<Group> binPackGroup(const Group& group, int max_size, GroupHash& group_hash, ChangesList& change_list);

vector<Group> clusterHierarch(const set<Cell>& cells, Grid& grid, GroupHash& group_hash, 
    const vector<int>& thresholds, const unordered_map<int, Flexoffer>& flexOffers);

void optimizeGroup(int group_id, Grid& grid, GroupHash& group_hash, const vector<int>& thresholds, 
                    ChangesList& change_list, const unordered_map<int, Flexoffer>& flexOffers);

void deltaProcess(const Flexoffer& offer, char action, GroupHash& group_hash);

#endif
