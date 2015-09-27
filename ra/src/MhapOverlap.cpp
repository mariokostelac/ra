#include "MhapOverlap.hpp"
#include <cassert>

using std::endl;
using std::ostream;

namespace MHAP {

  double MhapOverlap::score() const {

    double base_score = ((a_hi()-a_lo())/(double)a_len + (b_hi()-b_lo())/(double)b_len);

    if (jaccard_score > 0) {
      return base_score * jaccard_score;
    }

    return base_score;
  }

  Overlap* MhapOverlap::clone() const {
    return new MhapOverlap(*this);
  }

  void MhapOverlap::print(std::ostream& o) const {
    o << a() << " " << b() << " " << jaccard_score << " ";
    o << (double) (shared_minmers) << " ";
    o << (int) (a_rc) << " " << a_lo_ << " " << a_hi_ << " " << a_len << " ";
    o << (int) (b_rc) << " " << b_lo_ << " " << b_hi_ << " " << b_len << " ";
    o << endl;
  }

}
