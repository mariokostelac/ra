/*
* Utils.hpp
*
* Created on: Apr 25, 2015
*     Author: rvaser
*/

#pragma once

#include <sys/time.h>

#define ASSERT(expr, fmt, ...)\
    do {\
        if (!(expr)) {\
            fprintf(stderr, "[ERROR]: " fmt "\n", ##__VA_ARGS__);\
            exit(-1);\
        }\
    } while (0)

class Timer {
public:

    Timer();

    void start();
    void stop();
    void reset();

    void print(const char* message) const;

private:

    int paused_;
    long long time_;
    timeval timeval_;
};