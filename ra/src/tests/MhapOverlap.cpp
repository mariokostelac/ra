#include "gtest/gtest.h"
#include "../Read.hpp"
#include "../Overlap.hpp"

TEST(MhapOverlap, ReturnsRightInnie1) {

  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "C", "", 1);

  bool a_rc = false;
  int a_lo = 0, a_hi = 0;
  bool b_rc = false;
  int b_lo = 0, b_hi = 0;

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_EQ(false, overlap->is_innie());

  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(MhapOverlap, ReturnsRightInnie2) {

  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "C", "", 1);

  bool a_rc = false;
  int a_lo = 0, a_hi = 0;
  bool b_rc = true;
  int b_lo = 0, b_hi = 0;

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_EQ(true, overlap->is_innie());

  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(MhapOverlap, ReturnsRightBbounds1) {
  // AAACGT
  //    CGTTTT
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 5;
  bool b_rc = false;
  int b_lo = 0, b_hi = 2;

  auto read_a = new Read(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new Read(b_id, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_EQ(0, overlap->b_lo());
  ASSERT_EQ(2, overlap->b_hi());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(MhapOverlap, ReturnsRightBbounds2) {
  // AAACGT
  //    CGTTTT
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 5;
  bool b_rc = true;
  int b_lo = 3, b_hi = 5;

  auto read_a = new Read(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new Read(b_id, "read2", reverseComplement("CGTTTT").c_str(), "", 1);

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_EQ(3, overlap->b_lo());
  ASSERT_EQ(5, overlap->b_hi());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(MhapOverlap, ReturnsRightLengths1) {
  // AAACGT
  //    CGTTTT
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 6;
  bool b_rc = false;
  int b_lo = 0, b_hi = 3;

  auto read_a = new Read(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new Read(b_id, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_EQ(3, overlap->length(read_a->id()));
  ASSERT_EQ(3, overlap->length(read_b->id()));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(MhapOverlap, ReturnsRightLengthsRc) {
  // AAACGT
  //    CGTTTT
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 6;
  bool b_rc = true;
  int b_lo = 0, b_hi = 3;

  auto read_a = new Read(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new Read(b_id, "read2", reverseComplement("CGTTTT").c_str(), "", 1);

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_EQ(3, overlap->length(a_id));
  ASSERT_EQ(3, overlap->length(b_id));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(MhapOverlap, extractOverlappedPartNormal1) {
  // AAACGT
  //    CGTTTT
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 6;
  bool b_rc = false;
  int b_lo = 0, b_hi = 3;

  auto read_a = new Read(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new Read(b_id, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a_id).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(b_id).c_str());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(MhapOverlap, extractOverlappedPartNormal2) {
  //    |||
  // AAACGT
  //   CCGTTTT
  //   ||||
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 6;
  bool b_rc = false;
  int b_lo = 0, b_hi = 4;

  auto read_a = new Read(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new Read(b_id, "read2", "CCGTTTT", "", 1);

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a_id).c_str());
  ASSERT_STREQ("CCGT", overlap->extract_overlapped_part(b_id).c_str());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(MhapOverlap, extractOverlappedPartInnie1) {
  //    |||
  // AAACGT
  //    CGTTTT
  //    |||
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 3, a_hi = 6;
  bool b_rc = true;
  int b_lo = 0, b_hi = 3;

  auto read_a = new Read(a_id, "read1", "AAACGT", "", 1);
  auto read_b = new Read(b_id, "read2", reverseComplement("CGTTTT").c_str(), "", 1);

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a_id).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(b_id).c_str());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(MhapOverlap, extractOverlappedPartInnie2) {
  //    |||
  //    CGTTTT
  // AAACGT
  //    |||
  int a_id = 1, b_id = 2;
  bool a_rc = false;
  int a_lo = 0, a_hi = 3;
  bool b_rc = true;
  int b_lo = 3, b_hi = 6;

  auto read_a = new Read(a_id, "read1", "CGTTTT", "", 1);
  auto read_b = new Read(b_id, "read2", reverseComplement("AAACGT").c_str(), "", 1);

  auto overlap = new Overlap(read_a, a_lo, a_hi, a_rc,
      read_b, b_lo, b_hi, b_rc);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a_id).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(b_id).c_str());

  delete read_b;
  delete read_a;
  delete overlap;
}
