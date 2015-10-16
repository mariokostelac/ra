#include "gtest/gtest.h"
#include "../Depot.hpp"
#include "../AfgRead.hpp"
#include "../IO.hpp"

TEST(Depot, Creation) {
  auto depot = new Depot("depot_dummy");
  delete depot;
}

TEST(Depot, StoreLoad) {
  auto depot = new Depot("depot_dummy");

  ReadSet reads = { new AfgRead(124, "read124", "CNGTTTTNCGTGTGNNNNCCCCGTGTGTGTGT", "", 1.7777) };

  depot->store_reads(reads);

  auto read1 = depot->load_read(0);

  ASSERT_EQ(reads.front()->getId(), read1->getId());
  ASSERT_STREQ(reads.front()->getName().c_str(), read1->getName().c_str());
  ASSERT_STREQ(reads.front()->getSequence().c_str(), read1->getSequence().c_str());
  ASSERT_STREQ(reads.front()->getQuality().c_str(), read1->getQuality().c_str());
  ASSERT_EQ(reads.front()->getCoverage(), read1->getCoverage());

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
      ASSERT_EQ(reads[i]->getId(), reads2[i]->getId());
      ASSERT_STREQ(reads[i]->getName().c_str(), reads2[i]->getName().c_str());
      ASSERT_STREQ(reads[i]->getSequence().c_str(), reads2[i]->getSequence().c_str());
      ASSERT_STREQ(reads[i]->getQuality().c_str(), reads2[i]->getQuality().c_str());
      ASSERT_EQ(reads[i]->getCoverage(), reads2[i]->getCoverage());
  }

  delete depot;

  for (const auto& it: reads2) delete it;
  for (const auto& it: reads) delete it;
}
