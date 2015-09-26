#include "gtest/gtest.h"
#include "../ra.hpp"

//void filterTransitiveOverlaps(std::vector<DovetailOverlap*>& dst, const std::vector<DovetailOverlap*>& overlaps,
    //int threadLen, bool view = true);
    //

TEST(FilterTransitives, SimpleTest1) {
  // CGGT
  //   GTCC
  //    TCCC
  auto read1 = new AfgRead(1, "read1", "CGGT", "", 1);
  auto read2 = new AfgRead(2, "read2", "GTCC", "", 1);
  auto read3 = new AfgRead(3, "read3", "TCCC", "", 1);

  auto one_two = new DovetailOverlap(1, 2, 2, 2, false);
  auto two_three = new DovetailOverlap(2, 3, 1, 1, false);
  auto one_three = new DovetailOverlap(1, 3, 3, 3, false);

  one_two->set_read_a(read1);
  one_two->set_read_b(read2);

  two_three->set_read_a(read2);
  two_three->set_read_b(read3);

  one_three->set_read_a(read1);
  one_three->set_read_b(read3);

  std::vector<DovetailOverlap*> src;
  src.push_back(one_two);
  src.push_back(two_three);
  src.push_back(one_three);

  std::vector<DovetailOverlap*> dst;

  filterTransitiveOverlaps(dst, src, 1);

  ASSERT_NE(dst.end(), std::find(dst.begin(), dst.end(), one_two));
  ASSERT_NE(dst.end(), std::find(dst.begin(), dst.end(), two_three));
  ASSERT_EQ(dst.end(), std::find(dst.begin(), dst.end(), one_three));
  ASSERT_EQ(2, dst.size());
}

//std::pair<int, int> calc_forced_hangs(uint32_t a_lo, uint32_t a_hi, uint32_t a_len, bool a_rc,
    //uint32_t b_lo, uint32_t b_hi, uint32_t b_len, bool b_rc);

TEST(CalcForcedHangs, SimpleTest1) {
  // -|-|>
  //  |-|->
  int a_len = 3, b_len = 3;
  int a_lo = 1, a_hi = 2;
  int b_lo = 0, b_hi = 1;
  bool a_rc = false, b_rc = false;
  auto hangs = calc_forced_hangs(a_lo, a_hi, a_len, a_rc, b_lo, b_hi, b_len, b_rc);

  ASSERT_EQ(1, hangs.first);
  ASSERT_EQ(1, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest2) {
  // -|---|>
  //  |---|->
  int a_len = 5, b_len = 5;
  int a_lo = 1, a_hi = 4;
  int b_lo = 0, b_hi = 3;
  bool a_rc = false, b_rc = false;
  auto hangs = calc_forced_hangs(a_lo, a_hi, a_len, a_rc, b_lo, b_hi, b_len, b_rc);

  ASSERT_EQ(1, hangs.first);
  ASSERT_EQ(1, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest3) {
  // --|--|>
  //  -|--|-->
  int a_len = 5, b_len = 6;
  int a_lo = 2, a_hi = 4;
  int b_lo = 1, b_hi = 3;
  bool a_rc = false, b_rc = false;
  auto hangs = calc_forced_hangs(a_lo, a_hi, a_len, a_rc, b_lo, b_hi, b_len, b_rc);

  ASSERT_EQ(1, hangs.first);
  ASSERT_EQ(2, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest4) {
  // --|--|--->
  //  -|--|-->
  int a_len = 8, b_len = 6;
  int a_lo = 2, a_hi = 4;
  int b_lo = 1, b_hi = 3;
  bool a_rc = false, b_rc = false;
  auto hangs = calc_forced_hangs(a_lo, a_hi, a_len, a_rc, b_lo, b_hi, b_len, b_rc);

  ASSERT_EQ(1, hangs.first);
  ASSERT_EQ(-1, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest5) {
  //  -|--|-->
  // --|--|--->
  int a_len = 6, b_len = 8;
  int a_lo = 1, a_hi = 3;
  int b_lo = 2, b_hi = 4;
  bool a_rc = false, b_rc = false;
  auto hangs = calc_forced_hangs(a_lo, a_hi, a_len, a_rc, b_lo, b_hi, b_len, b_rc);

  ASSERT_EQ(-1, hangs.first);
  ASSERT_EQ(1, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest6) {
  // --|--|>
  //  <|--|---
  int a_len = 5, b_len = 6;
  int a_lo = 2, a_hi = 4;
  int b_lo = 3, b_hi = 5;
  bool a_rc = false, b_rc = true;
  auto hangs = calc_forced_hangs(a_lo, a_hi, a_len, a_rc, b_lo, b_hi, b_len, b_rc);

  ASSERT_EQ(1, hangs.first);
  ASSERT_EQ(2, hangs.second);
}

TEST(CalcForcedHangs, SimpleTest7) {
  //  |--|>
  // <|--|---
  int a_len = 3, b_len = 6;
  int a_lo = 0, a_hi = 2;
  int b_lo = 3, b_hi = 5;
  bool a_rc = false, b_rc = true;
  auto hangs = calc_forced_hangs(a_lo, a_hi, a_len, a_rc, b_lo, b_hi, b_len, b_rc);

  ASSERT_EQ(-1, hangs.first);
  ASSERT_EQ(2, hangs.second);
}
