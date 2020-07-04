#include "timer.h"

Timer::Timer()
{
    start_ = std::chrono::high_resolution_clock::now();
}

//return time taken in ms
double Timer::stop()
{
    auto end = std::chrono::high_resolution_clock::now();

    auto start = 
        std::chrono::time_point_cast<std::chrono::microseconds>(start_).time_since_epoch().count();
    auto stop = 
        std::chrono::time_point_cast<std::chrono::microseconds>(end).time_since_epoch().count();

    auto duration = stop - start;
    double ms = duration * 0.001;

    return ms;
}
