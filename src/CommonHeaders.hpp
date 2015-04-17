/*
* CommonHeaders.hpp
*
* Created on: Apr 13, 2015
*     Author: rvaser
*/

#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <stack>

#define ASSERT(expr, fmt, ...)\
    do {\
        if (!(expr)) {\
            fprintf(stderr, "[ERROR]: " fmt "\n", ##__VA_ARGS__);\
            exit(-1);\
        }\
    } while (0)
