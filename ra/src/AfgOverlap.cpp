/*
* AfgOverlap.cpp
*
* Created on: May 14, 2015
*     Author: rvaser
*/

#include "AfgOverlap.hpp"

AfgOverlap::AfgOverlap(int a, int b, int length, int aHang, int bHang, bool innie) :
    DovetailOverlap(a, b), length_(length), aHang_(aHang), bHang_(bHang), innie_(innie) {
}

bool AfgOverlap::isUsingPrefix(int readId) const {

    if (readId == getA()) {
        if (aHang_ <= 0) return true;

    } else if (readId == getB()) {
        if (innie_ == false && aHang_ >= 0) return true;
        if (innie_ == true && bHang_ <= 0) return true;
    }

    return false;
}

bool AfgOverlap::isUsingSuffix(int readId) const {

    if (readId == getA()) {
        if (bHang_ >= 0) return true;

    } else if (readId == getB()) {
        if (innie_ == false && bHang_ <= 0) return true;
        if (innie_ == true && aHang_ >= 0) return true;
    }

    return false;
}


uint AfgOverlap::hangingLength(int readId) const {

    if (readId == getA()) return abs(aHang_);
    if (readId == getB()) return abs(bHang_);

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
    if (aHang_ > 0) {
      len -= aHang_;
    }
    if (bHang_ < 0) {
      len -= abs(bHang_);
    }

    return len;
}

int AfgOverlap::getLengthB() const {
    ASSERT(getReadB() != nullptr, "Overlap", "Read* b is nullptr");

    int len = getReadB()->getSequence().length();
    if (aHang_ < 0) {
      len -= abs(aHang_);
    }
    if (bHang_ > 0) {
      len -= bHang_;
    }

    return len;
}

DovetailOverlap* AfgOverlap::clone() const {
    return new AfgOverlap(getA(), getB(), length_, aHang_, bHang_, innie_);
}

void AfgOverlap::print(std::ostream& o) const {
  o << "{OVL" << std::endl;
  o << "adj:" << (innie_ ? 'I' : 'N') << std::endl;
  o << "rds:" << getA() << "," << getB() << std::endl;
  o << "ahg:" << aHang_ << std::endl;
  o << "bhg:" << bHang_ << std::endl;
  o << "scr:" << getScore() << std::endl;
  o << "}" << std::endl;
}

