#ifndef HELPERFUNCTION_H
#define HELPERFUNCTION_H

#include <vector>
#include "flexoffer.h"
#include "group.h"

struct MBR {
    int min_est_hour; // Minimum earliest start hour
    int max_est_hour; // Maximum earliest start hour
    int min_lst_hour; // Minimum latest start hour
    int max_lst_hour; // Maximum latest start hour
};

void createMBR(const Group& group, MBR& mbr);
bool exceedsThreshold(const MBR& mbr, int est_threshold, int lst_threshold);

// Now includes max_group_size
void clusterGroup(std::vector<Group>& groups, int est_threshold, int lst_threshold, int max_group_size);

#endif // HELPERFUNCTION_H
