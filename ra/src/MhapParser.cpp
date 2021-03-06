
#include "MhapParser.hpp"

namespace MHAP {

  int read_overlaps(OverlapSet& overlaps, const ReadSet& reads, istream& input) {
    int read = 0;

    uint32_t a_id;
    uint32_t b_id;
    double jaccard_score;
    double shared_minmers;
    uint32_t a_rc;
    uint32_t a_lo;
    uint32_t a_hi;
    uint32_t a_len;
    uint32_t b_rc;
    uint32_t b_lo;
    uint32_t b_hi;
    uint32_t b_len;

    while (true) {
      input >> a_id >> b_id >> jaccard_score >> shared_minmers;
      input >> a_rc >> a_lo >> a_hi >> a_len;
      input >> b_rc >> b_lo >> b_hi >> b_len;

      // update params to fit Overlap
      a_hi += 1;
      if (b_rc) {
          auto tmp = b_lo;
          b_lo = b_len - (b_hi + 1);
          b_hi = b_len - tmp;
      } else {
          b_hi += 1;
      }

      auto overlap = new Overlap(
          reads[a_id - 1], a_lo, a_hi, a_rc,
          reads[b_id - 1], b_lo, b_hi, b_rc
      );

      if (input.fail()) {
        break;
      }

      overlaps.push_back(overlap);
      read++;
    }

    return read;
  }
}
