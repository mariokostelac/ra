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

#include "DovetailOverlap.hpp"
#include "Read.hpp"
#include "CommonHeaders.hpp"

class AfgOverlap: public DovetailOverlap {
public:

    AfgOverlap(int a, int b, int length, int aHang, int bHang, bool innie);
    ~AfgOverlap() {}

    int length() const;

    int length(int read_id) const;

    double quality() const {
        return 1;
    }

    // checks whether the start of read is contained in overlap
    // - respects direction of read (important for reverse complements)!
    bool isUsingPrefix(int readId) const;

    // checks whether the end of read is contained in overlap
    // - respects direction of read (important for reverse complements)!
    bool isUsingSuffix(int readId) const;

    uint hangingLength(int readId) const;

    DovetailOverlap* clone() const;

    void print(std::ostream& str) const;

private:

    int length_;
    bool innie_;

    int length_a() const;
    int length_b() const;
};
