#include "gtest/gtest.h"
#include "../Read.hpp"
#include "../DovetailOverlap.hpp"

TEST(DovetailOverlap, ReturnsRightAHang) {
  auto overlap = new DovetailOverlap(1, 2, 3, 4, false, -1, -1);
  ASSERT_EQ(3, overlap->a_hang());
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightBHang) {
  auto overlap = new DovetailOverlap(1, 2, 3, -4, false, -1, -1);
  ASSERT_EQ(-4, overlap->b_hang());
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightAId) {
  auto overlap = new DovetailOverlap(1, 2, 3, -4, false, -1, -1);
  ASSERT_EQ(1, overlap->a());
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightBId) {
  auto overlap = new DovetailOverlap(1, 2, 3, -4, false, -1, -1);
  ASSERT_EQ(2, overlap->b());
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightInnie1) {
  auto overlap = new DovetailOverlap(1, 2, 3, -4, false, -1 , -1);
  ASSERT_EQ(false, overlap->innie());
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightInnie2) {
  auto overlap = new DovetailOverlap(1, 2, 3, -4, true, -1, -1);
  ASSERT_EQ(true, overlap->innie());
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLengths1) {
  // AAACGT
  //    CGTTTT
  auto overlap = new DovetailOverlap(1, 2, 3, 3, false, -1, -1);

  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(3, overlap->length(read_a->id()));
  ASSERT_EQ(3, overlap->length(read_b->id()));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLengths2) {
  //    CGTTTT
  // AAACGT
  auto overlap = new DovetailOverlap(2, 1, -3, -3, false, -1, -1);

  auto read_b = new Read(1, "read1", "AAACGT", "", 1);
  auto read_a = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(3, overlap->length(read_a->id()));
  ASSERT_EQ(3, overlap->length(read_b->id()));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLengthsIfHasInsertions) {
  // |||
  // AAAC.GT
  //    CGTTTT
  //       |||
  auto overlap = new DovetailOverlap(1, 2, 3, 3, false, -1, -1);

  auto read_a = new Read(1, "read1", "AAACNGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(4, overlap->length(read_a->id()));
  ASSERT_EQ(3, overlap->length(read_b->id()));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLengthsIfHasInsertions2) {
  // |||
  // AAACGT
  //    C.GTTTT
  //        |||
  auto overlap = new DovetailOverlap(1, 2, 3, 3, false, -1, -1);

  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CNGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(3, overlap->length(read_a->id()));
  ASSERT_EQ(4, overlap->length(read_b->id()));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLowerBoundaries) {
  // AAACGT
  //    CGTTTT
  auto overlap = new DovetailOverlap(1, 2, 3, 3, false, -1, -1);

  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(3, overlap->a_lo());
  ASSERT_EQ(0, overlap->b_lo());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLowerBoundaries2) {
  //    CGTTTT
  // AAACGT
  auto overlap = new DovetailOverlap(2, 1, -3, -3, false, -1, -1);

  auto read2 = new Read(1, "read1", "AAACGT", "", 1);
  auto read1 = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read2);
  overlap->set_read_b(read1);

  ASSERT_EQ(0, overlap->a_lo());
  ASSERT_EQ(3, overlap->b_lo());

  delete read1;
  delete read2;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLowerBoundaries3) {
  // AAAC.GT
  //    CGTTTT
  auto overlap = new DovetailOverlap(1, 2, 3, 3, false, -1, -1);

  auto read_a = new Read(1, "read1", "AAACNGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(3, overlap->a_lo());
  ASSERT_EQ(0, overlap->b_lo());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLowerBoundaries4) {
  // AAACGT
  //    C.GTTTT
  auto overlap = new DovetailOverlap(1, 2, 3, 4, false, -1, -1);

  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CNGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(3, overlap->a_lo());
  ASSERT_EQ(0, overlap->b_lo());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHigherBoundaries) {
  // AAACGT
  //    CGTTTT
  auto overlap = new DovetailOverlap(1, 2, 3, 3, false, -1, -1);

  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(read_a->length(), overlap->a_hi());
  ASSERT_EQ(3, overlap->b_hi());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHigherBoundaries2) {
  //    CGTTTT
  // AAACGT
  auto overlap = new DovetailOverlap(2, 1, -3, -3, false, -1, -1);

  auto read2 = new Read(1, "read1", "AAACGT", "", 1);
  auto read1 = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read2);
  overlap->set_read_b(read1);

  ASSERT_EQ(3, overlap->a_hi());
  ASSERT_EQ(read1->length(), overlap->b_hi());

  delete read1;
  delete read2;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHigherBoundaries3) {
  // AAAC.GT
  //    CGTTTT
  auto overlap = new DovetailOverlap(1, 2, 3, 3, false, -1, -1);

  auto read1 = new Read(1, "read1", "AAACNGT", "", 1);
  auto read2 = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read1);
  overlap->set_read_b(read2);

  ASSERT_EQ(read1->length(), overlap->a_hi());
  ASSERT_EQ(3, overlap->b_hi());

  delete read1;
  delete read2;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHigherBoundaries4) {
  // AAACGT
  //    C.GTTTT
  auto overlap = new DovetailOverlap(1, 2, 3, 4, false, -1, -1);

  auto read1 = new Read(1, "read1", "AAACGT", "", 1);
  auto read2 = new Read(2, "read2", "CNGTTTT", "", 1);

  overlap->set_read_a(read1);
  overlap->set_read_b(read2);

  ASSERT_EQ(read1->length(), overlap->a_hi());
  ASSERT_EQ(3, overlap->b_hi());

  delete read1;
  delete read2;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHangingLength) {
  // ---|-->
  //    |--|--->
  auto overlap = new DovetailOverlap(1, 2, 3, 4, false, -1, -1);
  ASSERT_EQ(3, overlap->hanging_length(1));
  ASSERT_EQ(4, overlap->hanging_length(2));
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHangingLength2) {
  //    |--|--->
  // ---|-->
  auto overlap = new DovetailOverlap(1, 2, -3, -4, false, -1, -1);
  ASSERT_EQ(4, overlap->hanging_length(1));
  ASSERT_EQ(3, overlap->hanging_length(2));
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHangingLength3) {
  // ---|--|-->
  //    |--|
  auto overlap = new DovetailOverlap(1, 2, 3, -3, false, -1, -1);
  ASSERT_EQ(6, overlap->hanging_length(1));
  ASSERT_EQ(0, overlap->hanging_length(2));
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHangingLength4) {
  //    |--|
  // ---|--|-->
  auto overlap = new DovetailOverlap(1, 2, -3, 3, false, -1, -1);
  ASSERT_EQ(0, overlap->hanging_length(1));
  ASSERT_EQ(6, overlap->hanging_length(2));
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightSuffixPrefixValues) {
  // AAACGT
  //    CGTTTT
  auto a = 1;
  auto b = 2;
  auto overlap = new DovetailOverlap(a, b, 3, 3, false, -1, -1);

  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read_a);
  overlap->set_read_b(read_b);

  ASSERT_EQ(false, overlap->is_using_prefix(a));
  ASSERT_EQ(true, overlap->is_using_suffix(a));
  ASSERT_EQ(true, overlap->is_using_prefix(b));
  ASSERT_EQ(false, overlap->is_using_suffix(b));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightSuffixPrefixValues2) {
  //    CGTTTT
  // AAACGT
  auto a = 2;
  auto b = 1;
  auto overlap = new DovetailOverlap(a, b, -3, -3, false, -1, -1);

  auto read1 = new Read(1, "read1", "AAACGT", "", 1);
  auto read2 = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read2);
  overlap->set_read_b(read1);

  ASSERT_EQ(true, overlap->is_using_prefix(a));
  ASSERT_EQ(false, overlap->is_using_suffix(a));
  ASSERT_EQ(false, overlap->is_using_prefix(b));
  ASSERT_EQ(true, overlap->is_using_suffix(b));

  delete read2;
  delete read1;
  delete overlap;
}

// TODO: add tests for innie
TEST(DovetailOverlap, extractOverlappedPartNormal1) {
  //    CGTTTT
  // AAACGT
  auto a = 2;
  auto b = 1;
  auto overlap = new DovetailOverlap(a, b, -3, -3, false, -1, -1);

  auto read1 = new Read(1, "read1", "AAACGT", "", 1);
  auto read2 = new Read(2, "read2", "CGTTTT", "", 1);

  overlap->set_read_a(read2);
  overlap->set_read_b(read1);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(b).c_str());

  delete read2;
  delete read1;
  delete overlap;
}

TEST(DovetailOverlap, extractOverlappedPartNormal2) {
  //    |||
  // AAACGT
  //   CCGTTTT
  //   ||||
  auto a = 1;
  auto b = 2;
  auto overlap = new DovetailOverlap(a, b, 3, 3, false, -1, -1);

  auto read1 = new Read(1, "read1", "AAACGT", "", 1);
  auto read2 = new Read(2, "read2", "CCGTTTT", "", 1);

  overlap->set_read_a(read1);
  overlap->set_read_b(read2);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a).c_str());
  ASSERT_STREQ("CCGT", overlap->extract_overlapped_part(b).c_str());

  delete read2;
  delete read1;
  delete overlap;
}

TEST(DovetailOverlap, extractOverlappedPartInnie1) {
  //    |||
  // AAACGT
  //    CGTTTT
  //    |||
  auto a = 1;
  auto b = 2;
  auto overlap = new DovetailOverlap(a, b, 3, 3, true, -1, -1);

  auto read1 = new Read(1, "read1", "AAACGT", "", 1);
  auto read2 = new Read(2, "read2", reverseComplement("CGTTTT").c_str(), "", 1);

  overlap->set_read_a(read1);
  overlap->set_read_b(read2);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(b).c_str());

  delete read2;
  delete read1;
  delete overlap;
}

TEST(DovetailOverlap, extractOverlappedPartInnie2) {
  //    |||
  //    CGTTTT
  // AAACGT
  //    |||
  auto a = 1;
  auto b = 2;
  auto overlap = new DovetailOverlap(a, b, -3, -3, true, -1, -1);

  auto read1 = new Read(1, "read1", "CGTTTT", "", 1);
  auto read2 = new Read(2, "read2", reverseComplement("AAACGT").c_str(), "", 1);

  overlap->set_read_a(read1);
  overlap->set_read_b(read2);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(a).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(b).c_str());

  delete read2;
  delete read1;
  delete overlap;
}
