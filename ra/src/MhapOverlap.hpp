
#ifndef _MHAP_OVERLAP_H
#define _MHAP_OVERLAP_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include "Overlap.hpp"

namespace MHAP {

  class MhapOverlap: public Overlap {
    public:

      MhapOverlap(uint32_t a_id, uint32_t b_id, double jaccard_score, uint32_t shared_minmers,
          bool a_rc, uint32_t a_lo, uint32_t a_hi, uint32_t a_len,
          bool b_rc, uint32_t b_lo, uint32_t b_hi, uint32_t b_len)
        : Overlap(a_id, b_id, a_rc != b_rc), jaccard_score(jaccard_score), shared_minmers(shared_minmers),
        a_rc(a_rc), a_lo(a_lo), a_hi(a_hi), a_len(a_len),
        b_rc(b_rc), b_lo(b_lo), b_hi(b_hi), b_len(b_len) {

          assert(!a_rc);
        }

      ~MhapOverlap() {};

      int length() const;

      int length(int read_id) const;

      double score() const;

      double quality() const;

      Overlap* clone() const;

      void print(std::ostream& str) const;

    private:
      double jaccard_score;
      uint32_t shared_minmers;
      bool a_rc;
      uint32_t a_lo;
      uint32_t a_hi;
      uint32_t a_len;
      bool b_rc;
      uint32_t b_lo;
      uint32_t b_hi;
      uint32_t b_len;
  };
}

#endif
