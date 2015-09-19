#include "MhapOverlap.hpp"
#include <cassert>

#define THRESHOLD 0.08

using std::endl;
using std::ostream;

namespace MHAP {

  double MhapOverlap::getScore() const {

    double base_score = ((a_hi-a_lo)/(double)a_len + (b_hi-b_lo)/(double)b_len);

    if (jaccard_score > 0) {
      return base_score * jaccard_score;
    }

    return base_score;
  }

  double MhapOverlap::getQuality() const {
    return jaccard_score;
  }

  bool MhapOverlap::isUsingPrefix(int read_id) const {

    if (read_id == (int) getA()) {
      if (getAHang() <= 0) return true;

    } else if (read_id == (int) getB()) {
      if (b_rc == false && getAHang() >= 0) return true;
      if (b_rc == true && getBHang() <= 0) return true;
    }

    return false;
  }

  bool MhapOverlap::isUsingSuffix(int read_id) const {

    if (read_id == (int) getA()) {
      if (getBHang() >= 0) return true;

    } else if (read_id == (int) getB()) {
      if (b_rc == false && getBHang() <= 0) return true;
      if (b_rc == true && getAHang() >= 0) return true;
    }

    return false;
  }

  bool MhapOverlap::isReallyUsingPrefix(int read_id) const {

    if (read_id == getA()) {
      return a_lo <= THRESHOLD * a_len;
    } else if (read_id == getB()) {
      return b_lo <= THRESHOLD * b_len;
    }

    assert(false);
  }

  bool MhapOverlap::isReallyUsingSuffix(int read_id) const {

    if (read_id == getA()) {
      return a_hi >= a_len * (1 - THRESHOLD);
    } else if (read_id == getB()) {
      return b_hi >= b_len * (1 - THRESHOLD);
    }

    assert(false);
  }

  int MhapOverlap::getLength(int read_id) const {
    if (read_id == (int) getA()) {
      return getLengthA();
    } else if (read_id == (int) getB()) {
      return getLengthB();
    }

    assert(false);
  }

  int MhapOverlap::confirmedLength(const int read_id) const {
      assert(read_id == getA() || read_id == getB());

      if (read_id == getA()) {
        return a_hi - a_lo - 1;
      }

      return b_hi - b_lo - 1;
  }

  int MhapOverlap::getLengthA() const {
    return a_hi - a_lo - 1;
  }

  int MhapOverlap::getLengthB() const {
    return b_hi - b_lo - 1;
  }

  int MhapOverlap::getLength() const {
    return (getLength(getA()) + getLength(getB()))/2;
  }

  uint MhapOverlap::hangingLength(int r_id) const {
    assert(r_id == getA() || r_id == getB());
    if (r_id == getA()) {
      return hangingLengthA();
    }
    return hangingLengthB();
  }

  uint32_t MhapOverlap::hangingLengthA() const {
    return a_len - getLength(getA());
  }

  uint32_t MhapOverlap::hangingLengthB() const {
    return b_len - getLength(getB());
  }

  bool MhapOverlap::isInnie() const {
    return b_rc;
  }

  int MhapOverlap::getAHang() const {
    if (!a_rc && !b_rc) {
      // -----|------|---->
      //  ah -|------|---->
      //
      // -ah -|------|---->
      // -----|------|---->
      return a_lo - b_lo;
    } else if (!a_rc && b_rc) {
      // -----|------|---->
      //  ah <|------|-----
      //
      // -ah -|------|---->
      // <----|------|-----
      return a_lo - (b_len - b_hi);
    }

    assert(false);
  }

  int MhapOverlap::getBHang() const {
    if (!a_rc && !b_rc) {
      //     -|------|-> bh
      // -----|------|------>
      //
      // -----|------|------>
      //     -|------|-> -bh
      int b_after = b_len - b_hi;
      int a_after = a_len - a_hi;
      return b_after - a_after;
    } else if (!a_rc && b_rc) {
      //     -|------|-> bh
      // <----|------|-------
      //
      // -----|------|------>
      //     <|------|-- -bh
      int a_after = a_len - a_hi;
      int b_after = b_lo;
      return b_after - a_after;
    }

    assert(false);
  }

  Overlap* MhapOverlap::clone() const {
    auto copy = new MhapOverlap(getA(), getB(), jaccard_score, shared_minmers,
          a_rc, a_lo, a_hi, a_len,
          b_rc, b_lo, b_hi, b_len);

    copy->setReadA(getReadA());
    copy->setReadB(getReadB());

    return copy;
  }

  void MhapOverlap::print(std::ostream& o) const {
    o << getA() << " " << getB() << " " << jaccard_score << " ";
    o << (double) (shared_minmers) << " ";
    o << (int) (a_rc) << " " << a_lo << " " << a_hi << " " << a_len << " ";
    o << (int) (b_rc) << " " << b_lo << " " << b_hi << " " << b_len << " ";
    o << endl;
  }

}
