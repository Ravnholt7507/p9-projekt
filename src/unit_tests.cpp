#include <vector>
#include <iostream>

#include "../include/unit_test.h"
#include "../include/tec.h"

using namespace std;

int flexoffer();
int tec();

int runUnitTests(){
    int parsing = 0;
    int error = 1;

    cout << "Running all tests:\n";
    if(flexoffer()){
       cout << "Flexoffer unit test passed.\n";
    } else return error;
    if(tec()){
       cout << "Tec flexoffer unit test passed.\n";
    } else return error;

    return parsing;
}

int flexoffer(){
    vector<TimeSlice> testProfile{{0, 1}};
    Flexoffer testOffer(1, 1, 2, 3, testProfile, 1);

    if(testOffer.get_offer_id() != 1){
        cout << "Flexoffer unit test has failed. Flexoffers offer id was "
             << testOffer.get_offer_id() << " but was expected to be 1.\n";
        return 0;
    }
    if(testOffer.get_est() != 1){
        cout << "Flexoffer unit test has failed. Flexoffers earliest start time was " 
             << testOffer.get_est() << " but was expected to be 1.\n";
        return 0;
    }
    if(testOffer.get_lst() != 2){
        cout << "Flexoffer unit test has failed. Flexoffers latest start time was "
             << testOffer.get_lst() << " but was expected to be 2.\n";
        return 0;
    }
    if(testOffer.get_et() != 3){
        cout << "Flexoffer unit test has failed. Flexoffers latest end time was "
             << testOffer.get_et() << " but was expected to be 3.\n";
        return 0;
    }
    if(testOffer.get_profile()[0].min_power != 0){
        cout << "Flexoffer unit test has failed. Flexoffers profile[0] was "
             << testOffer.get_profile()[0].min_power << " but was expected to be 0.\n";
        return 0;
    }
    if(testOffer.get_profile()[0].max_power != 1){
        cout << "Flexoffer unit test has failed. Flexoffers profile[1] was "
             << testOffer.get_profile()[0].max_power << " but was expected to be 1.\n";
        return 0;
    }
    if(testOffer.get_duration() != 1){
        cout << "Flexoffer unit test has failed. Flexoffers duration was "
             << testOffer.get_et() << " but was expected to be 1.\n";
        return 0;
    }

    return 1;
};

int tec(){
    vector<TimeSlice> testProfile{{0, 1}};
    Tec_flexoffer testOffer(0, 1, 1, 1, 2, 3, testProfile, 1);

    if(testOffer.get_min_overall_kw() != 0){
        cout << "TEC unit test has failed. TECs minimum kwh overall was "
             << testOffer.get_min_overall_kw() << " but was expected to be 0.\n";
        return 0;
    }
    if(testOffer.get_max_overall_kw() != 1){
        cout << "TEC unit test has failed. TECs maximum kwh overall was "
             << testOffer.get_max_overall_kw() << " but was expected to be 1.\n";
        return 0;
    }
    if(testOffer.get_offer_id() != 1){
        cout << "TEC unit test has failed. TECs offer id was " 
             << testOffer.get_offer_id() << " but was expected to be 1.\n";
        return 0;
    }
    if(testOffer.get_est() != 1){
        cout << "TEC unit test has failed. TECs earliest start time was "
             << testOffer.get_est() << " but was expected to be 1.\n";
        return 0;
    }
    if(testOffer.get_lst() != 2){
        cout << "TEC unit test has failed. TECs latest start time was "
             << testOffer.get_lst() << " but was expected to be 2.\n";
        return 0;
    }
    if(testOffer.get_et() != 3){
        cout << "TEC unit test has failed. TECs latest end time was " 
             << testOffer.get_et() << " but was expected to be 3.\n";
        return 0;
    }
    if(testOffer.get_profile()[0].min_power != 0){
        cout << "TEC unit test has failed. TECs profile[0] was " 
             << testOffer.get_profile()[0].min_power << " but was expected to be 0.\n";
        return 0;
    }
    if(testOffer.get_profile()[0].max_power != 1){
        cout << "TEC unit test has failed. TECs profile[1] was " 
             << testOffer.get_profile()[0].max_power << " but was expected to be 1.\n";
        return 0;
    }
    if(testOffer.get_duration() != 1){
        cout << "TEC unit test has failed. TECs duration was "
             << testOffer.get_et() << " but was expected to be 1.\n";
        return 0;
    }

    return 1;
};
