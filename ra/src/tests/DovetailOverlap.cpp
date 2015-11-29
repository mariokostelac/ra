#include "gtest/gtest.h"
#include "../Read.hpp"
#include "../Overlap.hpp"

TEST(DovetailOverlap, ReturnsRightAHang) {
  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "B", "", 1);
  auto overlap = new Overlap(read_a, 3, read_b, 4, false);
  ASSERT_EQ(3, overlap->a_hang());
  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(DovetailOverlap, ReturnsRightBHang) {
  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "B", "", 1);
  auto overlap = new Overlap(read_a, 3, read_b, -4, false);
  ASSERT_EQ(-4, overlap->b_hang());
  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(DovetailOverlap, ReturnsRightInnie1) {
  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "B", "", 1);
  auto overlap = new Overlap(read_a, 3, read_b, -4, false);
  ASSERT_EQ(false, overlap->is_innie());
  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(DovetailOverlap, ReturnsRightInnie2) {
  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "B", "", 1);
  auto overlap = new Overlap(read_a, 3, read_b, -4, true);
  ASSERT_EQ(true, overlap->is_innie());
  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(DovetailOverlap, ReturnsRightLengths1) {
  // AAACGT
  //    CGTTTT
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, false);

  ASSERT_EQ(3, overlap->length(read_a->id()));
  ASSERT_EQ(3, overlap->length(read_b->id()));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLengths2) {
  //    CGTTTT
  // AAACGT
  auto read_b = new Read(1, "read1", "AAACGT", "", 1);
  auto read_a = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, -3, read_b, -3, false);

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
  auto read_a = new Read(1, "read1", "AAACNGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, false);

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
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CNGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, false);

  ASSERT_EQ(3, overlap->length(read_a->id()));
  ASSERT_EQ(4, overlap->length(read_b->id()));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLowerBoundaries) {
  // AAACGT
  //    CGTTTT
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, false);

  ASSERT_EQ(3, overlap->a_lo());
  ASSERT_EQ(0, overlap->b_lo());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLowerBoundaries2) {
  //    CGTTTT
  // AAACGT
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, -3, read_b, -3, false);

  ASSERT_EQ(0, overlap->a_lo());
  ASSERT_EQ(3, overlap->b_lo());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLowerBoundaries3) {
  // AAAC.GT
  //    CGTTTT
  auto read_a = new Read(1, "read1", "AAACNGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, false);

  ASSERT_EQ(3, overlap->a_lo());
  ASSERT_EQ(0, overlap->b_lo());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightLowerBoundaries4) {
  // AAACGT
  //    C.GTTTT
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CNGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 4, false);

  ASSERT_EQ(3, overlap->a_lo());
  ASSERT_EQ(0, overlap->b_lo());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHigherBoundaries) {
  // AAACGT
  //    CGTTTT
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, false);

  ASSERT_EQ(read_a->length(), overlap->a_hi());
  ASSERT_EQ(3, overlap->b_hi());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHigherBoundaries2) {
  //    CGTTTT
  // AAACGT
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, -3, read_b, -3, false);

  ASSERT_EQ(3, overlap->a_hi());
  ASSERT_EQ(read_b->length(), overlap->b_hi());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHigherBoundaries3) {
  // AAAC.GT
  //    CGTTTT
  auto read_a = new Read(1, "read1", "AAACNGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, false);

  ASSERT_EQ(read_a->length(), overlap->a_hi());
  ASSERT_EQ(3, overlap->b_hi());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHigherBoundaries4) {
  // AAACGT
  //    C.GTTTT

  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CNGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 4, false);

  ASSERT_EQ(read_a->length(), overlap->a_hi());
  ASSERT_EQ(3, overlap->b_hi());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightHangingLength) {
  // ---|-->
  //    |--|--->
  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "A", "", 1);
  auto overlap = new Overlap(read_a, 3, read_b, 4, false);
  ASSERT_EQ(3, overlap->hanging_length(read_a->id()));
  ASSERT_EQ(4, overlap->hanging_length(read_b->id()));
  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(DovetailOverlap, ReturnsRightHangingLength2) {
  //    |--|--->
  // ---|-->
  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "A", "", 1);
  auto overlap = new Overlap(read_a, -3, read_b, -4, false);
  ASSERT_EQ(4, overlap->hanging_length(read_a->id()));
  ASSERT_EQ(3, overlap->hanging_length(read_b->id()));
  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(DovetailOverlap, ReturnsRightHangingLength3) {
  // ---|--|-->
  //    |--|
  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "A", "", 1);
  auto overlap = new Overlap(read_a, 3, read_b, -3, false);
  ASSERT_EQ(6, overlap->hanging_length(read_a->id()));
  ASSERT_EQ(0, overlap->hanging_length(read_b->id()));
  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(DovetailOverlap, ReturnsRightHangingLength4) {
  //    |--|
  // ---|--|-->
  auto read_a = new Read(1, "1", "A", "", 1);
  auto read_b = new Read(2, "2", "A", "", 1);
  auto overlap = new Overlap(read_a, -3, read_b, 3, false);
  ASSERT_EQ(0, overlap->hanging_length(read_a->id()));
  ASSERT_EQ(6, overlap->hanging_length(read_b->id()));
  delete overlap;
  delete read_b;
  delete read_a;
}

TEST(DovetailOverlap, ReturnsRightSuffixPrefixValues) {
  // AAACGT
  //    CGTTTT
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, false);

  ASSERT_EQ(false, overlap->is_using_prefix(read_a->id()));
  ASSERT_EQ(true, overlap->is_using_suffix(read_a->id()));
  ASSERT_EQ(true, overlap->is_using_prefix(read_b->id()));
  ASSERT_EQ(false, overlap->is_using_suffix(read_b->id()));

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, ReturnsRightSuffixPrefixValues2) {
  //    CGTTTT
  // AAACGT
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_b, -3, read_a, -3, false);

  ASSERT_EQ(true, overlap->is_using_prefix(read_b->id()));
  ASSERT_EQ(false, overlap->is_using_suffix(read_b->id()));
  ASSERT_EQ(false, overlap->is_using_prefix(read_a->id()));
  ASSERT_EQ(true, overlap->is_using_suffix(read_a->id()));

  delete read_b;
  delete read_a;
  delete overlap;
}

// TODO: add tests for innie
TEST(DovetailOverlap, extractOverlappedPartNormal1) {
  //    CGTTTT
  // AAACGT
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CGTTTT", "", 1);

  auto overlap = new Overlap(read_b, -3, read_a, -3, false);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(read_a->id()).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(read_b->id()).c_str());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, extractOverlappedPartNormal2) {
  //    |||
  // AAACGT
  //   CCGTTTT
  //   ||||
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", "CCGTTTT", "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, false);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(read_a->id()).c_str());
  ASSERT_STREQ("CCGT", overlap->extract_overlapped_part(read_b->id()).c_str());

  delete read_b;
  delete read_a;
  delete overlap;
}

TEST(DovetailOverlap, extractOverlappedPartInnie1) {
  //    |||
  // AAACGT
  //    CGTTTT
  //    |||
  auto read_a = new Read(1, "read1", "AAACGT", "", 1);
  auto read_b = new Read(2, "read2", reverseComplement("CGTTTT").c_str(), "", 1);

  auto overlap = new Overlap(read_a, 3, read_b, 3, true);

  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(read_a->id()).c_str());
  ASSERT_STREQ("CGT", overlap->extract_overlapped_part(read_b->id()).c_str());

  delete read_b;
  delete read_a;
  delete overlap;
}
