/*!
 * @file Overlap.cpp
 *
 * @brief Overlap class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 14, 2015
 */


#include "ReadIndex.hpp"

const double EPSILON = 0.15;
const double ALPHA = 3;

static inline bool doubleEq(double x, double y, double eps) {
    return y <= x + eps && x <= y + eps;
}

double Overlap::score() const {
  return (length(a())/(double)read_a()->getLength() +
      length(b())/(double)read_b()->getLength());
}

uint32_t Overlap::length(uint32_t read_id) const {
  assert(read_id == a() || read_id == b());

  if (read_id == a()) {
    return a_hi() - a_lo();
  }

  return b_hi() - b_lo();
}

uint32_t Overlap::length() const {
  return (length(a()) + length(b()))/2;
}

std::string Overlap::extract_overlapped_part(uint32_t read_id) const {
  assert(read_id == a() || read_id == b());
  assert(read_a() != nullptr && read_b() != nullptr);

  if (read_id == a()) {
    // [lo, hi>
    int lo = a_lo();
    int hi = a_hi();

    return std::string(read_a()->getSequence()).substr(lo, hi - lo);
  } else {
    int lo = b_lo();
    int hi = b_hi();

    if (innie()) {
      return reverse_complement(std::string(read_b()->getSequence())).substr(lo, hi - lo);
    }

    return std::string(read_b()->getSequence()).substr(lo, hi - lo);
  }
}
