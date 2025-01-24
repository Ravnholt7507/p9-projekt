#include "../include/helperfunctions.h"
#include "../include/solver.h"
#include "../include/inc_aggregation.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <vector>
#include <limits>


IncAggregator::IncAggregator(){
}

void IncAggregator::init(int est_th, int lst_th, int max_gr_size, Alignments alignment, AggMode Aggmode){
    est_threshold = est_th;
    lst_threshold = lst_th;
    max_group_size = max_gr_size;
    align = alignment;
    mode = Aggmode;

    nextGroupId = 1;
    batchOffers.clear();
    IncrGroups.clear();
    mbrs.clear();
}

void IncAggregator::setMode(AggMode Mode){
    mode = Mode;
}



void IncAggregator::updateMBR(const Fo_Group &grp, MBR &mbr)
{
    mbr.minEst = 999999;
    mbr.maxEst = -1;
    mbr.minLst = 999999;
    mbr.maxLst = -1;

    for (auto &fo : grp.getFlexOffers()){
        int e = fo.get_est_hour();
        int l = fo.get_lst_hour();
        if (e < mbr.minEst) mbr.minEst = e;
        if (e > mbr.maxEst) mbr.maxEst = e;
        if (l < mbr.minLst) mbr.minLst = l;
        if (l > mbr.maxLst) mbr.maxLst = l;
    }
}


bool IncAggregator::canMerge(const MBR &mbr, const Flexoffer &fo){
    int e = fo.get_est_hour();
    int l = fo.get_lst_hour();

    int mergedMinE = min(mbr.minEst, e);
    int mergedMaxE = max(mbr.maxEst, e);
    int mergedMinL = min(mbr.minLst, l);
    int mergedMaxL = max(mbr.maxLst, l);

    if ((mergedMaxE - mergedMinE) > est_threshold) return false;
    if ((mergedMaxL - mergedMinL) > lst_threshold) return false;
    return true;
}

void IncAggregator::IncInsert(const Flexoffer &fo){
    
    for (auto &g : IncrGroups){
        auto it = mbrs.find(g.getGroupId());
        if ((int)g.getFlexOffers().size() > max_group_size){
            continue;
        }
        if (canMerge(it -> second, fo)){
            g.addFlexOffer(fo);
            updateMBR(g, it->second);
            return;
        }
    }

    // create new group for flexOffer if it does not fit into any current groups
    Fo_Group newG(nextGroupId++);
    newG.addFlexOffer(fo);
    IncrGroups.push_back(newG);

    MBR newMBR;
    updateMBR(newG, newMBR);
    mbrs[newG.getGroupId()] = newMBR;
}


void IncAggregator::processDelta(const FOdelta &delta){
    if (mode == AggMode::BATCH){
        batchOffers.push_back(delta.fo);
    }

    else if (mode == AggMode::INCREMENTAL){
        if (delta.type == AggType::INSERT){
            IncInsert(delta.fo);
        }
    }
}
vector<AggregatedFlexOffer> IncAggregator::finalize(){
    if (mode == AggMode::BATCH) {
        vector<AggregatedFlexOffer> results = nToMAggregation(batchOffers, est_threshold, lst_threshold, max_group_size, align, 1);
        return results;
    }

    else {
        vector<AggregatedFlexOffer> finalAggs;
        finalAggs.reserve(IncrGroups.size());
        for (auto &g : IncrGroups) {
            finalAggs.push_back(g.createAggregatedOffer(align));
        } 
        return finalAggs;
    }
}






//////////////////////
// For TEC
//////////////////////
IncAggregatorTec::IncAggregatorTec(){}

void IncAggregatorTec::init(int est_th, int lst_th, int max_g, Alignments alignment, AggMode m)
{
    est_threshold=est_th;
    lst_threshold=lst_th;
    max_group_size=max_g;
    mode=m;
    align = alignment;
    nextGroupId=1;
    batchOffers.clear();
    incrGroups.clear();
    mbrs.clear();
}

void IncAggregatorTec::setMode(AggMode M){
    mode=M;
}

void IncAggregatorTec::updateMBR(const Tec_Group &grp, MBR &mb)
{
    mb.minEst=999999; mb.maxEst=-1;
    mb.minLst=999999; mb.maxLst=-1;

    for(auto &fo : grp.getFlexOffers()){
        int e=fo.get_est_hour();
        int l=fo.get_lst_hour();
        if(e<mb.minEst) mb.minEst=e;
        if(e>mb.maxEst) mb.maxEst=e;
        if(l<mb.minLst) mb.minLst=l;
        if(l>mb.maxLst) mb.maxLst=l;
    }
}

bool IncAggregatorTec::canMerge(const MBR &mb, const Tec_flexoffer &fo)
{
    int e=fo.get_est_hour();
    int l=fo.get_lst_hour();
    int mergedMinE=std::min(mb.minEst,e);
    int mergedMaxE=std::max(mb.maxEst,e);
    int mergedMinL=std::min(mb.minLst,l);
    int mergedMaxL=std::max(mb.maxLst,l);

    if((mergedMaxE-mergedMinE)>est_threshold)return false;
    if((mergedMaxL-mergedMinL)>lst_threshold)return false;
    return true;
}

void IncAggregatorTec::doIncInsert(const Tec_flexoffer &fo)
{
    for(auto &g: incrGroups){
        auto it=mbrs.find(g.getGroupId());
        if((int)g.getFlexOffers().size()>=max_group_size) continue;
        if(canMerge(it->second, fo)){
            g.addFlexOffer(fo);
            updateMBR(g, it->second);
            return;
        }
    }

    // else new group
    Tec_Group newG(nextGroupId++);
    newG.addFlexOffer(fo);
    incrGroups.push_back(newG);
    MBR m;
    updateMBR(newG,m);
    mbrs[newG.getGroupId()] = m;
}

void IncAggregatorTec::processDelta(const TecDelta &delta)
{
    if(mode==AggMode::BATCH){
        if(delta.type==AggType::INSERT){
            batchOffers.push_back(delta.fo);
        }
    }
    else {
        if(delta.type==AggType::INSERT){
            doIncInsert(delta.fo);
        } else {
            // remove if needed
        }
    }
}

vector<AggregatedFlexOffer> IncAggregatorTec::finalize()
{
    if(mode==AggMode::BATCH){
        auto aggOffers = nToMAggregation(batchOffers, est_threshold, lst_threshold, max_group_size, align,1);
        return aggOffers;
    }
    else {
        std::vector<AggregatedFlexOffer> finalAggs;
        finalAggs.reserve(incrGroups.size());
        for(auto &g : incrGroups){
            finalAggs.push_back(g.createAggregatedOffer(Alignments::start));
        }
        return finalAggs;
    }
}