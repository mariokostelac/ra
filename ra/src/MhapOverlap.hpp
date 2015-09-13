
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
        : Overlap(a_id, b_id), jaccard_score(jaccard_score), shared_minmers(shared_minmers),
        a_rc(a_rc), a_lo(a_lo), a_hi(a_hi), a_len(a_len),
        b_rc(b_rc), b_lo(b_lo), b_hi(b_hi), b_len(b_len) {

          assert(!a_rc);
        }

      ~MhapOverlap() {};

      int getLength() const;

      int getLength(int read_id) const;

      int getAHang() const;

      int getBHang() const;

      double getScore() const;

      double getQuality() const;

      bool isInnie() const;

      // checks whether the start of read is contained in overlap
      // - respects direction of read (important for reverse complements)!
      // at least one of isUsingPrefix/isUsingSuffix returns true (heuristics included)
      bool isUsingPrefix(int readId) const;

      // checks whether the end of read is contained in overlap
      // - respects direction of read (important for reverse complements)!
      // at least one of isUsingPrefix/isUsingSuffix returns true (heuristics included)
      bool isUsingSuffix(int readId) const;

      uint hangingLength(int readId) const;

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

      uint32_t hangingLengthA() const;
      uint32_t hangingLengthB() const;

      // returns if overlap really uses prefix of given read
      bool isReallyUsingPrefix(int readId) const;

      // returns if overlap really uses suffix of given read
      bool isReallyUsingSuffix(int readId) const;

      int getLengthA() const;

      int getLengthB() const;
  };
}

#endif
