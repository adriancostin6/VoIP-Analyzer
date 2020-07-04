#ifndef TIMER_H
#define TIMER_H
#include <iostream>
#include <memory>

#include <chrono>

class Timer
{
    public:
        Timer();
        double stop();
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

#endif

