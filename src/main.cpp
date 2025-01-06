#include <iostream>
#include <vector>
#include <variant> 
#include <string>

#include "../include/aggregation.h"
#include "../include/solver.h"
#include "../include/generator.h"
#include "../include/evaluation.h" 
#include "../include/helperfunctions.h"
#include "../include/tec.h"
#include "../include/unit_test.h"


using namespace std;

int main(int argc, char *argv[]) {
    if(argc > 1 && find(argv, argv+argc, "--run_tests")){
        return runUnitTests();
    }
    string filename = "../data/spotprices.csv";
    vector<double> spotPrices = readSpotPricesFromCSV(filename);

    string evDataFile = "../data/ev_data.csv"; // We get both normal and tec offers here
    vector<variant<Flexoffer, Tec_flexoffer>> parsedNormalOffers = parseEVDataToFlexOffers(evDataFile, 0);
    vector<variant<Flexoffer, Tec_flexoffer>> parsedTecOffers = parseEVDataToFlexOffers(evDataFile, 1);

    vector<DFO> dfos = parseEVDataToDFO(evDataFile, 5);
    vector<Flexoffer> fos;
    vector<Tec_flexoffer> tec_fos;

    for (auto &offer : parsedNormalOffers) {
        if (holds_alternative<Flexoffer>(offer)) {
            fos.push_back(get<Flexoffer>(offer));
        }
    }
    for (auto &offer : parsedTecOffers) {
        if (holds_alternative<Tec_flexoffer>(offer)) {
            tec_fos.push_back(get<Tec_flexoffer>(offer));
        }
    }

    runAggregationScenarios(fos, tec_fos, dfos, spotPrices);
    return 0;
}

// still needs to add the DFO to main properly, and then get FCR working properly