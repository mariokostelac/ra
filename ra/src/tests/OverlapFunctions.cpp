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
