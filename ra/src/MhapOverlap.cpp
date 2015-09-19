#include "MhapOverlap.hpp"
#include <cassert>

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

  int MhapOverlap::getLength(int read_id) const {
    assert(read_id == getA() || read_id == getB());

    if (read_id == (int) getA()) {
      return a_hi - a_lo - 1;
    }

    return a_hi - a_lo - 1;
  }

  int MhapOverlap::getLength() const {
    return (getLength(getA()) + getLength(getB()))/2;
  }

  bool MhapOverlap::isInnie() const {
    return b_rc;
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
