#include "gtest/gtest.h"
#include "../ra.hpp"

TEST(FilterTransitives, SimpleTest1) {
  // CGGT
  //   GTCC
  //    TCCC
  auto read1 = new Read(1, "read1", "CGGT", "", 1);
  auto read2 = new Read(2, "read2", "GTCC", "", 1);
  auto read3 = new Read(3, "read3", "TCCC", "", 1);

  auto one_two = new Overlap(read1, 2, read2, 2, false);
  auto two_three = new Overlap(read2, 1, read3, 1, false);
  auto one_three = new Overlap(read1, 3, read3, 3, false);

  std::vector<Overlap*> src;
  src.push_back(one_two);
  src.push_back(two_three);
  src.push_back(one_three);

  std::vector<Overlap*> dst;
  filterTransitiveOverlaps(dst, src, 1);

  ASSERT_NE(dst.end(), std::find(dst.begin(), dst.end(), one_two));
  ASSERT_NE(dst.end(), std::find(dst.begin(), dst.end(), two_three));
  ASSERT_EQ(dst.end(), std::find(dst.begin(), dst.end(), one_three));
  ASSERT_EQ(2, dst.size());

  delete one_three;
  delete two_three;
  delete one_two;

  delete read3;
  delete read2;
  delete read1;
}

TEST(CalcForcedHangs, SimpleTest1) {
  // -|-|>
  //  |-|->
  int a_len = 3, b_len = 3;
  int a_lo = 1, a_hi = 2;
  int b_lo = 0, b_hi = 1;
  auto hangs = calculateForcedHangs(a_lo, a_hi, a_len, b_lo, b_hi, b_len);

  ASSERT_EQ(1, hangs.first);
  ASSERT_EQ(1, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest2) {
  // -|---|>
  //  |---|->
  int a_len = 5, b_len = 5;
  int a_lo = 1, a_hi = 4;
  int b_lo = 0, b_hi = 3;
  auto hangs = calculateForcedHangs(a_lo, a_hi, a_len, b_lo, b_hi, b_len);

  ASSERT_EQ(1, hangs.first);
  ASSERT_EQ(1, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest3) {
  // --|--|>
  //  -|--|-->
  int a_len = 5, b_len = 6;
  int a_lo = 2, a_hi = 4;
  int b_lo = 1, b_hi = 3;
  auto hangs = calculateForcedHangs(a_lo, a_hi, a_len, b_lo, b_hi, b_len);

  ASSERT_EQ(1, hangs.first);
  ASSERT_EQ(2, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest4) {
  // --|--|--->
  //  -|--|-->
  int a_len = 8, b_len = 6;
  int a_lo = 2, a_hi = 4;
  int b_lo = 1, b_hi = 3;
  auto hangs = calculateForcedHangs(a_lo, a_hi, a_len, b_lo, b_hi, b_len);

  ASSERT_EQ(1, hangs.first);
  ASSERT_EQ(-1, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest5) {
  //  -|--|-->
  // --|--|--->
  int a_len = 6, b_len = 8;
  int a_lo = 1, a_hi = 3;
  int b_lo = 2, b_hi = 4;
  auto hangs = calculateForcedHangs(a_lo, a_hi, a_len, b_lo, b_hi, b_len);

  ASSERT_EQ(-1, hangs.first);
  ASSERT_EQ(1, hangs.second);
}

TEST(DovetailOverlap, extractOverlappedPartInnie2) {
  //    |||
  //    CGTTTT
  // AAACGT
  //    |||
  auto read_a = new Read(1, "read1", "CGTTTT", "", 1);
  auto read_b = new Read(2, "read2", reverseComplement("AAACGT").c_str(), "", 1);

  auto overlap = new Overlap(read_a, -3, read_b, -3, true);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(read_a->id()).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(read_b->id()).c_str());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(ForcedDovetailOverlap, SuffixPrefixDovetailingAllowsGapsInQuery) {

  // CGTTTCCCC
  //   |||
  //  GTTTCCCCAA
  auto read_a = new Read(1, "read1", "CGTTTCCCC", "", 1);
  auto read_b = new Read(2, "read2", "GTTTCCCCAA", "", 1);

  Overlap* overlap = new Overlap(read_a, 2, 5, false, read_b, 1, 4, false);
  Overlap* dovetail_overlap = forcedDovetailOverlap(overlap, true);

  ASSERT_EQ(1, dovetail_overlap->a_lo());
  ASSERT_EQ(read_a->length(), dovetail_overlap->a_hi());

  ASSERT_EQ(0, dovetail_overlap->b_lo());
  ASSERT_EQ(8, dovetail_overlap->b_hi());

  ASSERT_EQ(0, dovetail_overlap->err_rate());
  ASSERT_EQ(0, dovetail_overlap->orig_err_rate());

  delete read_b;
  delete read_a;
  delete overlap;
  delete dovetail_overlap;
}

TEST(ForcedDovetailOverlap, PrefixSuffixDovetailingAllowsGapsInQuery) {

  //   AAAACCC
  //    |||
  // GGAAAACC
  auto read_a = new Read(1, "read1", "AAAACCC", "", 1);
  auto read_b = new Read(2, "read2", "GGAAAACC", "", 1);

  Overlap* overlap = new Overlap(read_a, 1, 4, false, read_b, 3, 6, false);
  Overlap* dovetail_overlap = forcedDovetailOverlap(overlap, true);

  ASSERT_EQ(0, dovetail_overlap->a_lo());
  ASSERT_EQ(6, dovetail_overlap->a_hi());

  ASSERT_EQ(2, dovetail_overlap->b_lo());
  ASSERT_EQ(read_b->length(), dovetail_overlap->b_hi());

  ASSERT_EQ(0, dovetail_overlap->err_rate());
  ASSERT_EQ(0, dovetail_overlap->orig_err_rate());

  delete read_b;
  delete read_a;
  delete overlap;
  delete dovetail_overlap;
}

TEST(ForcedDovetailOverlap, PrefixPrefixDovetailingAllowsGapsInQuery) {

  //   AAAACCC
  //    |||
  // GGAAAACC
  auto read_a = new Read(1, "read1", "AAAACCC", "", 1);
  auto read_b = new Read(2, "read2", reverseComplement("GGAAAACC"), "", 1);

  Overlap* overlap = new Overlap(read_a, 1, 4, false, read_b, 3, 6, true);
  Overlap* dovetail_overlap = forcedDovetailOverlap(overlap, true);

  ASSERT_EQ(0, dovetail_overlap->a_lo());
  ASSERT_EQ(read_a->length() - 1, dovetail_overlap->a_hi());

  ASSERT_EQ(2, dovetail_overlap->b_lo());
  ASSERT_EQ(read_b->length(), dovetail_overlap->b_hi());

  ASSERT_EQ(0, dovetail_overlap->err_rate());
  ASSERT_EQ(0, dovetail_overlap->orig_err_rate());

  delete read_b;
  delete read_a;
  delete overlap;
  delete dovetail_overlap;
}

TEST(ForcedDovetailOverlap, SuffixSuffixDovetailingAllowsGapsInQuery) {

  // AAACCCC
  //    |||
  //   ACCCCGGG
  auto read_a = new Read(1, "read1", "AAACCCC", "", 1);
  auto read_b = new Read(2, "read2", reverseComplement("ACCCCGGG"), "", 1);

  Overlap* overlap = new Overlap(read_a, 3, 6, false, read_b, 1, 4, true);
  Overlap* dovetail_overlap = forcedDovetailOverlap(overlap, true);

  ASSERT_EQ(2, dovetail_overlap->a_lo());
  ASSERT_EQ(read_a->length(), dovetail_overlap->a_hi());

  ASSERT_EQ(0, dovetail_overlap->b_lo());
  ASSERT_EQ(5, dovetail_overlap->b_hi());

  ASSERT_EQ(0, dovetail_overlap->err_rate());
  ASSERT_EQ(0, dovetail_overlap->orig_err_rate());

  delete read_b;
  delete read_a;
  delete overlap;
  delete dovetail_overlap;
}
