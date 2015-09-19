/*
* AfgOverlap.hpp
*
*
* Created on: May 14, 2015
*     Author: rvaser
*/

#pragma once

#include "CommonHeaders.hpp"
#include "DovetailOverlap.hpp"
#include "Read.hpp"

class AfgOverlap: public DovetailOverlap {
public:

    AfgOverlap(int a, int b, int length, int aHang, int bHang, bool innie);
    ~AfgOverlap() {}

    // TODO
    double quality() const {
        return 1;
    }

    DovetailOverlap* clone() const;

    void print(std::ostream& str) const;

private:

    int length_;
    bool innie_;

    int length_a() const;
    int length_b() const;
};
