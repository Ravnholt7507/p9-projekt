#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <random>
#include <vector>
#include "flexoffers/flexoffer.h"
using namespace std;

int main(){
    int numOffers = 5;
    vector<Flexoffer> flexOffers = generateMultipleFlexOffers(numOffers);

    for (auto& flexOffer : flexOffers) {
        flexOffer.print_flexoffer();
    }

    return 0;
}

