#include "gtest/gtest.h"
#include "../Read.hpp"

TEST(Read, AcceptsN) {
  // C.GTTTT
  const char* expected = "CNGTTTT";
  auto read2 = new Read(2, "read2", expected, "", 1);

  ASSERT_STREQ(expected, read2->sequence().c_str());

  delete read2;
}

TEST(Read, Serialization) {

    const char* expected = "CNGTTTTNCGTGTGNNNNCCCCGTGTGTGTGT";
    auto read2 = new Read(124, "read3", expected, "", 1.7777);

    char* bytes;
    uint32_t bytes_length;
    read2->serialize(&bytes, &bytes_length);

    auto read1 = Read::deserialize(bytes);

    ASSERT_EQ(read1->id(), read2->id());
    ASSERT_STREQ(read1->name().c_str(), read2->name().c_str());
    ASSERT_STREQ(read1->sequence().c_str(), read2->sequence().c_str());
    ASSERT_STREQ(read1->quality().c_str(), read2->quality().c_str());
    ASSERT_EQ(read1->coverage(), read2->coverage());

    delete read1;
    delete[] bytes;
    delete read2;
}
