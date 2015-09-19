/*
* AfgOverlap.cpp
*
* Created on: May 14, 2015
*     Author: rvaser
*/

#include "AfgOverlap.hpp"

AfgOverlap::AfgOverlap(int a, int b, int length, int aHang, int bHang, bool innie) :
    DovetailOverlap(a, b, aHang, bHang, innie), length_(length), innie_(innie) {
}

DovetailOverlap* AfgOverlap::clone() const {
    return new AfgOverlap(a(), b(), length_, a_hang(), b_hang(), innie_);
}

void AfgOverlap::print(std::ostream& o) const {
  o << "{OVL" << std::endl;
  o << "adj:" << (innie_ ? 'I' : 'N') << std::endl;
  o << "rds:" << a() << "," << b() << std::endl;
  o << "ahg:" << a_hang() << std::endl;
  o << "bhg:" << b_hang() << std::endl;
  o << "scr:" << score() << std::endl;
  o << "}" << std::endl;
}

