#ifndef TIMER_H
#define TIMER_H
#include <iostream>
#include <memory>

#include <chrono>

//Class Timer
//
//Starts a timer and ends it in stop function and returns the value of time passed
//Parameters: 
//  - start_ : chrono::time_point<high_resolution_clock> (stores the start time)
//Methods:
//  - Timer() : constructor
//  - stop() : stops the timer and returns value of time passed
class Timer
{
    public:
        Timer();
        double stop();
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

#endif

