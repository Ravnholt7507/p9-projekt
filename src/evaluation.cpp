#include <iostream>
#include <variant>
#include <ctime>

#include "../include/aggregation.h"

using namespace std;

double calculateFlexibilityLoss(AggregatedFlexOffer offer){
    double originalTimeFlexibility = 0.0, aggregatedTimeFlexibility = 0.0;
    time_t lst;
    time_t est;
    for (unsigned int i=0; i < offer.get_individual_offers().size(); i++){
        if(holds_alternative<Flexoffer>(offer.get_individual_offers()[i])){
            lst = get<Flexoffer>(offer.get_individual_offers()[i]).get_lst();
            est = get<Flexoffer>(offer.get_individual_offers()[i]).get_est();
        }else if(holds_alternative<Tec_flexoffer>(offer.get_individual_offers()[i])){
            lst = get<Tec_flexoffer>(offer.get_individual_offers()[i]).get_lst();
            est = get<Tec_flexoffer>(offer.get_individual_offers()[i]).get_est();
        }
        originalTimeFlexibility += difftime(lst, est);
    }
    aggregatedTimeFlexibility = difftime(offer.get_aggregated_earliest(), offer.get_aggregated_earliest());
    return 1.0 - (aggregatedTimeFlexibility / originalTimeFlexibility);
}

void evaluateAggregatedFlexOffer(AggregatedFlexOffer offer) {
    double flexLoss = calculateFlexibilityLoss(offer);
    cout << " === AFO evalation ===" << "\n";
    cout << "Flexiblity loss: " << flexLoss << "\n";
}
