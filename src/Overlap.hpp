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

typedef std::shared_ptr<class Overlap> OverlapPtr;

class Overlap {
public:

    Overlap(int aId, int bId, int length, int aHang, int bHang, bool innie);
    ~Overlap() {}

    int getAId() const {
        return aId_;
    }

    int getBId() const {
        return bId_;
    }

    int getLength() const {
        return length_;
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

    int aId_;
    int bId_;
    int length_;
    int aHang_;
    int bHang_;
    bool innie_;
};

// path is used to cache the ReadIndex
void overlapReads(std::vector<OverlapPtr>& dst, std::vector<ReadPtr>& reads, int minOverlapLen,
    int threadLen, const char* path);
