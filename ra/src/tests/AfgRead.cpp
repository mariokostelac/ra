#include "gtest/gtest.h"
#include "../AfgRead.hpp"

TEST(AfgRead, AcceptsN) {
  // C.GTTTT
  const char* expected = "CNGTTTT";
  auto read2 = new AfgRead(2, "read2", expected, "", 1);

  ASSERT_STREQ(expected, read2->getSequence().c_str());
}
