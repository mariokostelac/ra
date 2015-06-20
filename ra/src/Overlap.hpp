/*
* Overlap.hpp
*
* Created on: May 14, 2015
*     Author: rvaser
*
* Filtering of contained and transitive overlaps was rewritten and modified from:
*     Github: https://github.com/mariokostelac/assembly-tools
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

    virtual ~Overlap(){};

    virtual int getA() const = 0;

    virtual void setA(int a) = 0;

    virtual int getB() const = 0;

    virtual void setB(int b) = 0;

    virtual int getLength() const = 0;

    virtual int getAHang() const = 0;

    virtual int getBHang() const = 0;

    virtual bool isInnie() const = 0;

    // checks whether the start of read is contained in overlap
    // - respects direction of read (important for reverse complements)!
    virtual bool isUsingPrefix(int readId) const = 0;

    // checks whether the end of read is contained in overlap
    // - respects direction of read (important for reverse complements)!
    virtual bool isUsingSuffix(int readId) const = 0;

    // checks whether this (o1) is transitive considering overlaps o2 and o3
    virtual bool isTransitive(const Overlap* o2, const Overlap* o3) const;

    virtual uint hangingLength(int readId) const = 0;

    virtual Overlap* clone() const = 0;

    virtual void print(std::ostream& str) const = 0;

    friend std::ostream& operator<<(std::ostream& str, Overlap const& data)
    {
      data.print(str);
      return str;
    }
};

// path is used to cache the ReadIndex
void overlapReads(std::vector<Overlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path);

// updates coverage of reads
void filterContainedOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps,
    std::vector<Read*>& reads, bool view = true);

void filterTransitiveOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps,
    int threadLen, bool view = true);

void updateOverlapIds(std::vector<Overlap*>& overlaps, std::vector<Read*>& reads);

