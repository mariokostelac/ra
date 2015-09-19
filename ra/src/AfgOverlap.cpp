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

    if (readId == getA()) {
        if (getAHang() <= 0) return true;

    } else if (readId == getB()) {
        if (innie_ == false && getAHang() >= 0) return true;
        if (innie_ == true && getBHang() <= 0) return true;
    }

    return false;
}

bool AfgOverlap::isUsingSuffix(int readId) const {

    if (readId == getA()) {
        if (getBHang() >= 0) return true;

    } else if (readId == getB()) {
        if (innie_ == false && getBHang() <= 0) return true;
        if (innie_ == true && getAHang() >= 0) return true;
    }

    return false;
}


uint AfgOverlap::hangingLength(int readId) const {

    if (readId == getA()) return abs(getAHang());
    if (readId == getB()) return abs(getBHang());

    ASSERT(false, "Overlap", "wrong read id");
}

int AfgOverlap::getLength() const {
    return (getLengthA() + getLengthB())/2;
}

int AfgOverlap::getLength(int read_id) const {
    if (read_id == getA()) {
        return getLengthA();
    } else if (read_id == getB()) {
        return getLengthB();
    }

    assert(false);
}

int AfgOverlap::getLengthA() const {
    ASSERT(getReadA() != nullptr, "Overlap", "Read* a is nullptr");

    int len = getReadA()->getSequence().length();
    if (getAHang() > 0) {
      len -= getAHang();
    }
    if (getBHang() < 0) {
      len -= abs(getBHang());
    }

    return len;
}

int AfgOverlap::getLengthB() const {
    ASSERT(getReadB() != nullptr, "Overlap", "Read* b is nullptr");

    int len = getReadB()->getSequence().length();
    if (getAHang() < 0) {
      len -= abs(getAHang());
    }
    if (getBHang() > 0) {
      len -= getBHang();
    }

    return len;
}

DovetailOverlap* AfgOverlap::clone() const {
    return new AfgOverlap(getA(), getB(), length_, getAHang(), getBHang(), innie_);
}

void AfgOverlap::print(std::ostream& o) const {
  o << "{OVL" << std::endl;
  o << "adj:" << (innie_ ? 'I' : 'N') << std::endl;
  o << "rds:" << getA() << "," << getB() << std::endl;
  o << "ahg:" << getAHang() << std::endl;
  o << "bhg:" << getBHang() << std::endl;
  o << "scr:" << getScore() << std::endl;
  o << "}" << std::endl;
}

