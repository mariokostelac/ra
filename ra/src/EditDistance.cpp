/*!
 * @file EditDistance.cpp
 *
 * @brief Edlib wrapper source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Jun 07, 2015
 */

#include "EditDistance.hpp"

#include "../vendor/edlib/edlib.h"

static unsigned char toUnsignedChar(char c) {

    unsigned char r;

    switch(c) {
        case 'A':
            r = 0;
            break;
        case 'T':
            r = 1;
            break;
        case 'G':
            r = 2;
            break;
        case 'C':
            r = 3;
            break;
        default:
            r = 4;
            break;
    }

    return r;
}

extern int editDistance(const std::string& queryStr, const std::string& targetStr) {

    if (queryStr.size() == 0) return targetStr.size();
    if (targetStr.size() == 0) return queryStr.size();

    unsigned char* query = new unsigned char[queryStr.size()];
    int queryLength = queryStr.size();

    for (int i = 0; i < queryLength; ++i) {
        query[i] = toUnsignedChar(queryStr[i]);
    }

    unsigned char* target = new unsigned char[targetStr.size()];
    int targetLength = targetStr.size();

    for (int i = 0; i < targetLength; ++i) {
        target[i] = toUnsignedChar(targetStr[i]);
    }

    int alphabetLength = 5;
    int k = -1;
    int mode = EDLIB_MODE_NW;
    bool findStartLocations = false;
    bool findAlignment = false;
    int score = 0;
    int* endLocations = nullptr; // dummy
    int* startLocations = nullptr; // dummy
    int numLocations = 0; // dummy;
    unsigned char* alignemnt = nullptr; // dummy
    int alignmentLength = 0; // dummy;

    edlibCalcEditDistance(query, queryLength, target, targetLength, alphabetLength,
        k, mode, findStartLocations, findAlignment, &score, &endLocations,
        &startLocations, &numLocations, &alignemnt, &alignmentLength);

    free(alignemnt);
    free(startLocations);
    free(endLocations);

    delete[] target;
    delete[] query;

    return score;
}

extern int32_t editDistanceSHW(const std::string& queryStr, int query_lo, const std::string& targetStr, int target_lo, int* query_best_end) {
    int queryLength = queryStr.size() - query_lo;
    int targetLength = targetStr.size() - target_lo;

    if (queryLength == 0) return 0;
    if (targetLength == 0) return queryLength;

    unsigned char* query = new unsigned char[queryLength];

    for (int i = 0; i < queryLength; ++i) {
        query[i] = toUnsignedChar(queryStr[i + query_lo]);
    }

    unsigned char* target = new unsigned char[targetLength];

    for (int i = 0; i < targetLength; ++i) {
        target[i] = toUnsignedChar(targetStr[i + target_lo]);
    }

    int alphabetLength = 5;
    int k = -1;
    int mode = EDLIB_MODE_SHW;
    bool findStartLocations = false;
    bool findAlignment = false;
    int score = 0;
    int* endLocations = nullptr; // dummy
    int* startLocations = nullptr; // dummy
    int numLocations = 0; // dummy;
    unsigned char* alignemnt = nullptr; // dummy
    int alignmentLength = 0; // dummy;

    edlibCalcEditDistance(query, queryLength, target, targetLength, alphabetLength,
        k, mode, findStartLocations, findAlignment, &score, &endLocations,
        &startLocations, &numLocations, &alignemnt, &alignmentLength);

    assert(numLocations > 0);
    *query_best_end = endLocations[0] + 1; // we like [lo, hi>

    free(alignemnt);
    free(startLocations);
    free(endLocations);

    delete[] target;
    delete[] query;

    return score;
}
