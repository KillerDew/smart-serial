#include "./include/clock/Std_clock.hpp"
#include <cstdio>
#include <iostream>
using namespace std;
int main() {
    
    Smart_serial::Clock::Std_clock clock;

    cout << "Waiting for 1 second ";
    clock.delay(1000U);
    cout << "Done" << endl;;
    printf("Elapsed time %fms", static_cast<double>(clock.millis())/1000);
    
    return 0;
}