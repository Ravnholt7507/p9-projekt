#include <iostream>
#include "flexoffer.h"
using namespace std;

Flexoffer::Flexoffer(int oi, int est, int lst, double *p, int d, int et){
        offer_id = oi;
        earliest_start_time = est;
        latest_start_time = lst;
        for(int i = 0; i < 24; i++) {
            profile[i] = p[i];
        }
        duration = d;
        end_time = et;
};

void Flexoffer::print_flexoffer(){
    cout << "Offer id: " << offer_id << endl << "Earliest start time: " << earliest_start_time << endl << "latest_start_time" << latest_start_time;
    cout << "Profile elements: ";
    for(int i = 0; i < 24; i++){
        std::cout << profile[i] << " ";
    }
    cout << endl << "Duration: " << duration << endl << "End time: " << end_time << endl;
}

