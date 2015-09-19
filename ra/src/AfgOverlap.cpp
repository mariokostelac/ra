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

bool AfgOverlap::isUsingPrefix(int readId) const {

    if (readId == a()) {
        if (a_hang() <= 0) return true;

    } else if (readId == b()) {
        if (innie_ == false && a_hang() >= 0) return true;
        if (innie_ == true && b_hang() <= 0) return true;
    }

    return false;
}

bool AfgOverlap::isUsingSuffix(int readId) const {

    if (readId == a()) {
        if (b_hang() >= 0) return true;

    } else if (readId == b()) {
        if (innie_ == false && b_hang() <= 0) return true;
        if (innie_ == true && a_hang() >= 0) return true;
    }

    return false;
}


uint AfgOverlap::hangingLength(int readId) const {

    if (readId == a()) return abs(a_hang());
    if (readId == b()) return abs(b_hang());

    ASSERT(false, "Overlap", "wrong read id");
}

int AfgOverlap::length() const {
    return (length_a() + length_b())/2;
}

int AfgOverlap::length(int read_id) const {
    assert(read_id == a() || read_id == b());

    if (read_id == a()) {
        return length_a();
    }

    return length_b();
}

int AfgOverlap::length_a() const {
    ASSERT(read_a() != nullptr, "Overlap", "Read* a is nullptr");

    int len = read_a()->getSequence().length();
    if (a_hang() > 0) {
      len -= a_hang();
    }
    if (b_hang() < 0) {
      len -= abs(b_hang());
    }

    return len;
}

int AfgOverlap::length_b() const {
    ASSERT(read_b() != nullptr, "Overlap", "Read* b is nullptr");

    int len = read_b()->getSequence().length();
    if (a_hang() < 0) {
      len -= abs(a_hang());
    }
    if (b_hang() > 0) {
      len -= b_hang();
    }

    return len;
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

