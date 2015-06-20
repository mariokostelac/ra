/*
* AfgOverlap.cpp
*
* Created on: May 14, 2015
*     Author: rvaser
*/

#include "ReadIndex.hpp"

void updateOverlapIds(std::vector<Overlap*>& overlaps, std::vector<Read*>& reads) {

    for (const auto& overlap : overlaps) {
        overlap->setA(reads[overlap->getA()]->getId());
        overlap->setB(reads[overlap->getB()]->getId());
    }
}
