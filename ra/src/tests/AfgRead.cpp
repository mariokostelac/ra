#include "gtest/gtest.h"
#include "../AfgRead.hpp"

TEST(AfgRead, AcceptsN) {
  // C.GTTTT
  const char* expected = "CNGTTTT";
  auto read2 = new AfgRead(2, "read2", expected, "", 1);

  ASSERT_STREQ(expected, read2->getSequence().c_str());

  delete read2;
}

TEST(AfgRead, Serialization) {

    const char* expected = "CNGTTTTNCGTGTGNNNNCCCCGTGTGTGTGT";
    auto read2 = new AfgRead(124, "read3", expected, "", 1.7777);

    char* bytes;
    uint32_t bytes_length;
    read2->serialize(&bytes, &bytes_length);

    auto read1 = AfgRead::deserialize(bytes);

    ASSERT_EQ(read1->getId(), read2->getId());
    ASSERT_STREQ(read1->getName().c_str(), read2->getName().c_str());
    ASSERT_STREQ(read1->getSequence().c_str(), read2->getSequence().c_str());
    ASSERT_STREQ(read1->getQuality().c_str(), read2->getQuality().c_str());
    ASSERT_EQ(read1->getCoverage(), read2->getCoverage());

    delete read1;
    delete[] bytes;
    delete read2;
}
