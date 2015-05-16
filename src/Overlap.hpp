/*
* Overlap.hpp
*
* Created on: May 14, 2015
*     Author: rvaser
*
* For explanation about terminology see: sourceforge.net/p/amos/mailman/message/19965222/
*/

#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

class Overlap {
public:

    Overlap(const Read* a, const Read* b, int length, int aHang, int bHang, bool innie);
    ~Overlap() {}

    int getLength() const {
        return length_;
    }

    void print();

private:

    const Read* a_;
    const Read* b_;
    int length_;
    int aHang_;
    int bHang_;
    bool innie_;
};

// path is used to cache the ReadIndex
void getOverlaps(std::vector<Overlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path);
