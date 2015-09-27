/*
* AfgOverlap.cpp
*
* Created on: May 14, 2015
*     Author: rvaser
*/

#include "AfgOverlap.hpp"

AfgOverlap::AfgOverlap(int a, int b, int length, int aHang, int bHang, bool innie) :
    DovetailOverlap(a, b, aHang, bHang, innie, -1, -1) {
}

AfgOverlap* AfgOverlap::clone() const {
  return new AfgOverlap(*this);
}
