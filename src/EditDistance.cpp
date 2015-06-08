/*
* EditDistance.cpp
*
* Created on: Jun 07, 2015
*     Author: rvaser
*/

#include "EditDistance.hpp"

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
    int* endLocations = NULL; // dummy
    int* startLocations = NULL; // dummy
    int numLocations = 0; // dummy;
    unsigned char* alignemnt = NULL; // dummy
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
