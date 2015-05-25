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

    Overlap(const Read* a, const Read* b, int aHang, int bHang, bool innie);
    ~Overlap() {}

    const Read* getReadA() const {
        return a_;
    }

    const Read* getReadB() const {
        return b_;
    }

    int getAHang() const {
        return aHang_;
    }

    int getBHang() const {
        return bHang_;
    }

    bool isInnie() const {
        return innie_;
    }

    void print();

private:

    const Read* a_;
    const Read* b_;
    int aHang_;
    int bHang_;
    bool innie_;
};

// path is used to cache the ReadIndex
void overlapReads(std::vector<Overlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path);
