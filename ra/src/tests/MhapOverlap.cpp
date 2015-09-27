#include "gtest/gtest.h"
#include "../AfgRead.hpp"
#include "../MhapOverlap.hpp"

using namespace MHAP;


TEST(MhapOverlap, ReturnsRightIds) {
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 0, a_hi = 0, a_len = 0;
  bool b_rc = false;
  int b_lo = 0, b_hi = 0, b_len = 0;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);
  ASSERT_EQ(a_id, overlap->a());
  ASSERT_EQ(b_id, overlap->b());
}

TEST(MhapOverlap, ReturnsRightInnie1) {
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 0, a_hi = 0, a_len = 0;
  bool b_rc = false;
  int b_lo = 0, b_hi = 0, b_len = 0;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);
  ASSERT_EQ(false, overlap->innie());
}

TEST(MhapOverlap, ReturnsRightInnie2) {
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 0, a_hi = 0, a_len = 0;
  bool b_rc = true;
  int b_lo = 0, b_hi = 0, b_len = 0;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);
  ASSERT_EQ(true, overlap->innie());
}

TEST(MhapOverlap, ReturnsRightBbounds1) {
  // AAACGT
  //    CGTTTT
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 5, a_len = 6;
  bool b_rc = false;
  int b_lo = 0, b_hi = 2, b_len = 6;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);

  auto read_a = new AfgRead(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new AfgRead(b_id, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(0, overlap->b_lo());
  ASSERT_EQ(3, overlap->b_hi());
}

TEST(MhapOverlap, ReturnsRightBbounds2) {
  // AAACGT
  //    CGTTTT
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 5, a_len = 6;
  bool b_rc = true;
  int b_lo = 3, b_hi = 5, b_len = 6;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);

  auto read_a = new AfgRead(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new AfgRead(b_id, "read2", reverse_complement("CGTTTT").c_str(), "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(0, overlap->b_lo());
  ASSERT_EQ(3, overlap->b_hi());
}

TEST(MhapOverlap, ReturnsRightLengths1) {
  // AAACGT
  //    CGTTTT
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 5, a_len = 6;
  bool b_rc = false;
  int b_lo = 0, b_hi = 2, b_len = 6;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);

  auto read_a = new AfgRead(1, "read1", "AAACGT", "", 1);
  auto read_b = new AfgRead(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(3, overlap->length(read_a->getId()));
  ASSERT_EQ(3, overlap->length(read_b->getId()));
}

TEST(MhapOverlap, ReturnsRightLengthsRc) {
  // AAACGT
  //    CGTTTT
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 5, a_len = 6;
  bool b_rc = true;
  int b_lo = 0, b_hi = 2, b_len = 6;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);

  auto read_a = new AfgRead(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new AfgRead(b_id, "read2", reverse_complement("CGTTTT").c_str(), "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(3, overlap->length(a_id));
  ASSERT_EQ(3, overlap->length(b_id));
}

TEST(MhapOverlap, extractOverlappedPartNormal1) {
  // AAACGT
  //    CGTTTT
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 5, a_len = 6;
  bool b_rc = false;
  int b_lo = 0, b_hi = 2, b_len = 6;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);

  auto read_a = new AfgRead(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new AfgRead(b_id, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a_id).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(b_id).c_str());
}

TEST(MhapOverlap, extractOverlappedPartNormal2) {
  //    |||
  // AAACGT
  //   CCGTTTT
  //   ||||
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 5, a_len = 6;
  bool b_rc = false;
  int b_lo = 0, b_hi = 3, b_len = 7;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);

  auto read_a = new AfgRead(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new AfgRead(b_id, "read2", "CCGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a_id).c_str());
  ASSERT_STREQ("CCGT", overlap->extract_overlapped_part(b_id).c_str());
}

TEST(MhapOverlap, extractOverlappedPartInnie1) {
  //    |||
  // AAACGT
  //    CGTTTT
  //    |||
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 5, a_len = 6;
  bool b_rc = true;
  int b_lo = 3, b_hi = 5, b_len = 6;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);

  auto read_a = new AfgRead(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new AfgRead(b_id, "read2", reverse_complement("CGTTTT").c_str(), "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a_id).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(b_id).c_str());
}

TEST(MhapOverlap, extractOverlappedPartInnie2) {
  //    |||
  //    CGTTTT
  // AAACGT
  //    |||
  double jaccard = 0;
  uint32_t shared_minmers = 0;
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 0, a_hi = 2, a_len = 6;
  bool b_rc = true;
  int b_lo = 0, b_hi = 2, b_len = 6;

  auto overlap = new MhapOverlap(a_id, b_id, jaccard, shared_minmers,
      a_rc, a_lo, a_hi, a_len,
      b_rc, b_lo, b_hi, b_len);

  auto read_a = new AfgRead(a_id, "read1", "CGTTTT", "", 1);
  auto read_b = new AfgRead(b_id, "read2", reverse_complement("AAACGT").c_str(), "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a_id).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(b_id).c_str());
}
