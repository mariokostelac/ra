/*
* Utils.cpp
*
* Created on: Apr 25, 2015
*     Author: rvaser
*/

#include <stdlib.h>
#include <stdio.h>

#include "Utils.hpp"

bool fileExists(const char* path) {
    struct stat buf;
    return stat(path, &buf) != -1;
}

Timer::Timer() :
    paused_(0), time_(0), timeval_() {
}

void Timer::start() {

    gettimeofday(&timeval_, NULL);
    paused_ = 0;
}

void Timer::stop() {

    if (paused_) return;

    timeval stop;
    gettimeofday(&stop, NULL);

    time_ += ((stop.tv_sec - timeval_.tv_sec) * 1000000L + stop.tv_usec) - timeval_.tv_usec;
    paused_ = 1;
}

void Timer::reset() {

    gettimeofday(&timeval_, NULL);
    time_ = 0;
    paused_ = 0;
}

void Timer::print(const char* location, const char* message) const {
    fprintf(stderr, "[%s][%s]: %.5lf s\n", location, message, time_ / (double) 1000000);
}
