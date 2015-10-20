#include "gtest/gtest.h"
#include "../Depot.hpp"
#include "../Read.hpp"
#include "../IO.hpp"

TEST(Depot, Creation) {
  auto depot = new Depot("depot_dummy");
  delete depot;
}

TEST(Depot, StoreLoad) {
  auto depot = new Depot("depot_dummy");

  ReadSet reads = { new Read(124, "read124", "CNGTTTTNCGTGTGNNNNCCCCGTGTGTGTGT", "", 1.7777) };

  depot->store_reads(reads);

  auto read1 = depot->load_read(0);

  ASSERT_EQ(reads.front()->id(), read1->id());
  ASSERT_STREQ(reads.front()->name().c_str(), read1->name().c_str());
  ASSERT_STREQ(reads.front()->sequence().c_str(), read1->sequence().c_str());
  ASSERT_STREQ(reads.front()->quality().c_str(), read1->quality().c_str());
  ASSERT_EQ(reads.front()->coverage(), read1->coverage());
  ASSERT_EQ(reads.front()->length(), read1->length());

  for (const auto& it: reads) delete it;

  delete read1;

  delete depot;
}

TEST(Depot, StoreHeavy) {

  ReadSet reads;
  readFastqReads(reads, "../examples/ERR430949.fastq");

  auto depot = new Depot("depot_dummy");

  depot->store_reads(reads);

  delete depot;

  for (const auto& it: reads) delete it;
}

TEST(Depot, LoadHeavy) {

  ReadSet reads;
  readFastqReads(reads, "../examples/ERR430949.fastq");

  auto depot = new Depot("depot_dummy");

  ReadSet reads2;
  depot->load_reads(reads2);

  ASSERT_EQ(reads.size(), reads2.size());

  for (uint32_t i = 0; i < reads.size(); ++i) {
      ASSERT_EQ(reads[i]->length(), reads2[i]->length());
      ASSERT_EQ(reads[i]->id(), reads2[i]->id());
      ASSERT_STREQ(reads[i]->name().c_str(), reads2[i]->name().c_str());
      ASSERT_STREQ(reads[i]->sequence().c_str(), reads2[i]->sequence().c_str());
      ASSERT_STREQ(reads[i]->quality().c_str(), reads2[i]->quality().c_str());
      ASSERT_EQ(reads[i]->coverage(), reads2[i]->coverage());
  }

  delete depot;

  for (const auto& it: reads2) delete it;
  for (const auto& it: reads) delete it;
}

TEST(Depot, LoadHeavyByNReads) {

    ReadSet reads;
    readFastqReads(reads, "../examples/ERR430949.fastq");

    auto depot = new Depot("depot_dummy");
    uint32_t n = 50;

    uint32_t total = 0;

    for (uint32_t i = 0; i < reads.size(); i += n) {

        ReadSet reads2;
        depot->load_reads(reads2, i, n);

        total += reads2.size();

        for (uint32_t j = 0; j < reads2.size(); ++j) {
            ASSERT_EQ(reads[i + j]->length(), reads2[j]->length());
            ASSERT_EQ(reads[i + j]->id(), reads2[j]->id());
            ASSERT_STREQ(reads[i + j]->name().c_str(), reads2[j]->name().c_str());
            ASSERT_STREQ(reads[i + j]->sequence().c_str(), reads2[j]->sequence().c_str());
            ASSERT_EQ(reads[i + j]->coverage(), reads2[j]->coverage());
            ASSERT_STREQ(reads[i + j]->quality().c_str(), reads2[j]->quality().c_str());
        }

        for (const auto& it: reads2) delete it;
    }

    ASSERT_EQ(reads.size(), total);

    delete depot;
    for (const auto& it: reads) delete it;
}

TEST(Depot, LoadHeavyBy1Read) {

    ReadSet reads;
    readFastqReads(reads, "../examples/ERR430949.fastq");

    auto depot = new Depot("depot_dummy");
    uint32_t n = 1;

    uint32_t total = 0;

    for (uint32_t i = 0; i < reads.size(); i += n) {

        ReadSet reads2;
        depot->load_reads(reads2, i, n);

        total += reads2.size();

        for (uint32_t j = 0; j < reads2.size(); ++j) {
            ASSERT_EQ(reads[i + j]->length(), reads2[j]->length());
            ASSERT_EQ(reads[i + j]->id(), reads2[j]->id());
            ASSERT_STREQ(reads[i + j]->name().c_str(), reads2[j]->name().c_str());
            ASSERT_STREQ(reads[i + j]->sequence().c_str(), reads2[j]->sequence().c_str());
            ASSERT_EQ(reads[i + j]->coverage(), reads2[j]->coverage());
            ASSERT_STREQ(reads[i + j]->quality().c_str(), reads2[j]->quality().c_str());
        }

        for (const auto& it: reads2) delete it;
    }

    ASSERT_EQ(reads.size(), total);

    delete depot;
    for (const auto& it: reads) delete it;
}
