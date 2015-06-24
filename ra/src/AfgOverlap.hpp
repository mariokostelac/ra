/*
* AfgOverlap.hpp
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

#include "Overlap.hpp"
#include "Read.hpp"
#include "CommonHeaders.hpp"

class AfgOverlap: public Overlap {
public:

    AfgOverlap(int a, int b, int length, int aHang, int bHang, bool innie);
    ~AfgOverlap() {}

    int getA() const {
        return a_;
    }

    void setA(int a) {
        a_ = a;
    }

    Read* getReadA() const {
        return ra_;
    }

    void setReadA(Read* read) {
        ra_ = read;
    }

    int getB() const {
        return b_;
    }

    void setB(int b) {
        b_ = b;
    }

    Read* getReadB() const {
        return rb_;
    }

    void setReadB(Read* read) {
        rb_ = read;
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

    uint hangingLength(int readId) const;

    Overlap* clone() const;

    void print(std::ostream& str) const;

private:

    Read* ra_;
    Read* rb_;
    int a_;
    int b_;
    int length_;
    int aHang_;
    int bHang_;
    bool innie_;
};
