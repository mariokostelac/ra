/*
* Overlap.hpp
*
* Created on: May 14, 2015
*     Author: rvaser
*
* Filtering of contained and transitive overlaps was rewritten from:
*     https://github.com/mariokostelac/assembly-tools
*
* Overlap types:
*     (found at sourceforge.net/p/amos/mailman/message/19965222/)
*
*     Normal:
*
*     read a  ---------------------------------->   bHang
*     read b      aHang  ----------------------------------->
*
*     read a     -aHang  ----------------------------------->
*     read b  ---------------------------------->  -bHang
*
*     Innie:
*
*     read a  ---------------------------------->   bHang
*     read b      aHang  <-----------------------------------
*
*     read a     -aHang  ----------------------------------->
*     read b  <----------------------------------  -bHang
*
*     Containment:
*
*     read a  ---------------------------------------------->
*     read b      aHang  ----------------------->  -bHang
*
*     read a     -aHang  ----------------------->   bHang
*     read b  ---------------------------------------------->
*
*     (same if read b is reversed)
*/

#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

class Overlap {
public:

    Overlap(int a, int b, int length, int aHang, int bHang, bool innie);
    ~Overlap() {}

    int getA() const {
        return a_;
    }

    int getB() const {
        return b_;
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

    // checks whether the start of read is contained in overlap
    // - respects direction of read (important for reverse complements)!
    bool isUsingPrefix(int readId) const;

    // checks whether the end of read is contained in overlap
    // - respects direction of read (important for reverse complements)!
    bool isUsingSuffix(int readId) const;

    // checks whether this (o1) is transitive considering overlaps o2 and o3
    bool isTransitive(const Overlap* o2, const Overlap* o3) const;

    int hang(int readId) const;

    Overlap* clone() const;

private:

    int a_;
    int b_;
    int length_;
    int aHang_;
    int bHang_;
    bool innie_;
};

// path is used to cache the ReadIndex
void overlapReads(std::vector<Overlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path);

void filterContainedOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps,
    bool view = true);

void filterTransitiveOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps,
    int threadLen, bool view = true);
