#include "gtest/gtest.h"
#include "../Depot.hpp"
#include "../Read.hpp"
#include "../Overlap.hpp"
#include "../OverlapFunctions.hpp"
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

TEST(Depot, StoreLoadHeavy) {

  ReadSet reads;
  readFastqReads(reads, "../examples/ERR430949.fastq");

  auto depot = new Depot("depot_dummy");
  depot->store_reads(reads);

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

TEST(Depot, StoreLoadHeavyByNReads) {

    ReadSet reads;
    readFastqReads(reads, "../examples/ERR430949.fastq");

    auto depot = new Depot("depot_dummy");
    depot->store_reads(reads);

    uint32_t n = 51;
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

TEST(Depot, StoreLoadHeavyBy1Read) {

    ReadSet reads;
    readFastqReads(reads, "../examples/ERR430949.fastq");

    auto depot = new Depot("depot_dummy");
    depot->store_reads(reads);

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

TEST(Depot, StoreLoadOverlap) {
  auto depot = new Depot("depot_dummy");

  ReadSet reads = { new Read(0, "read124", "CNGTTTTNCGTGTGNNNNCCCCGTGTGTGTGT", "", 1.7777),
    new Read(1, "read12", "ACTG", "", 10) };

  OverlapSet overlaps = { new Overlap(reads[0], 12, reads[1], -12, true) };

  depot->store_overlaps(overlaps);
  auto overlap1 = depot->load_overlap(0, reads);

  ASSERT_EQ(overlaps[0]->a(), overlap1->a());
  ASSERT_EQ(overlaps[0]->a_hang(), overlap1->a_hang());
  ASSERT_EQ(overlaps[0]->b(), overlap1->b());
  ASSERT_EQ(overlaps[0]->b_hang(), overlap1->b_hang());
  ASSERT_EQ(overlaps[0]->is_innie(), overlap1->is_innie());
  ASSERT_EQ(overlaps[0]->is_dovetail(), overlap1->is_dovetail());
  ASSERT_EQ(overlaps[0]->a_lo(), overlap1->a_lo());
  ASSERT_EQ(overlaps[0]->a_hi(), overlap1->a_hi());
  ASSERT_EQ(overlaps[0]->b_lo(), overlap1->b_lo());
  ASSERT_EQ(overlaps[0]->b_hi(), overlap1->b_hi());
  ASSERT_EQ(overlaps[0]->err_rate(), overlap1->err_rate());
  ASSERT_EQ(overlaps[0]->orig_err_rate(), overlap1->orig_err_rate());

  delete overlap1;
  for (const auto& it: overlaps) delete it;
  for (const auto& it: reads) delete it;
  delete depot;
}

TEST(Depot, StoreOverlapsHeavy) {

  ReadSet reads;
  readFastqReads(reads, "../examples/ERR430949.fastq");

  auto depot = new Depot("depot_dummy");

  OverlapSet overlaps;
  overlapReads(overlaps, reads, 75, 1, "depot_dummy/esa");

  depot->store_overlaps(overlaps);

  OverlapSet overlaps2;
  depot->load_overlaps(overlaps2, reads);

  ASSERT_EQ(overlaps.size(), overlaps2.size());
  for (uint32_t i = 0; i < overlaps.size(); ++i) {
      ASSERT_EQ(overlaps[i]->a(), overlaps2[i]->a());
      ASSERT_EQ(overlaps[i]->a_hang(), overlaps2[i]->a_hang());
      ASSERT_EQ(overlaps[i]->b(), overlaps2[i]->b());
      ASSERT_EQ(overlaps[i]->b_hang(), overlaps2[i]->b_hang());
      ASSERT_EQ(overlaps[i]->is_innie(), overlaps2[i]->is_innie());
      ASSERT_EQ(overlaps[i]->is_dovetail(), overlaps2[i]->is_dovetail());
      ASSERT_EQ(overlaps[i]->a_lo(), overlaps2[i]->a_lo());
      ASSERT_EQ(overlaps[i]->a_hi(), overlaps2[i]->a_hi());
      ASSERT_EQ(overlaps[i]->b_lo(), overlaps2[i]->b_lo());
      ASSERT_EQ(overlaps[i]->b_hi(), overlaps2[i]->b_hi());
      ASSERT_EQ(overlaps[i]->err_rate(), overlaps2[i]->err_rate());
      ASSERT_EQ(overlaps[i]->orig_err_rate(), overlaps2[i]->orig_err_rate());
  }

  for (const auto& it: overlaps2) delete it;
  for (const auto& it: overlaps) delete it;
  for (const auto& it: reads) delete it;
  delete depot;
}

TEST(Depot, StoreLoadOverlapsHeavyByNOverlaps) {

    ReadSet reads;
    readFastqReads(reads, "../examples/ERR430949.fastq");

    auto depot = new Depot("depot_dummy");

    OverlapSet overlaps;
    overlapReads(overlaps, reads, 75, 1, "depot_dummy/esa");

    depot->store_overlaps(overlaps);

    OverlapSet overlaps2;

    uint32_t n = 51;
    uint32_t total = 0;

    for (uint32_t i = 0; i < overlaps.size(); i += n) {

        OverlapSet overlaps2;
        depot->load_overlaps(overlaps2, i, n, reads);

        total += overlaps2.size();

        for (uint32_t j = 0; j < overlaps2.size(); ++j) {
            ASSERT_EQ(overlaps[i + j]->a(), overlaps2[j]->a());
            ASSERT_EQ(overlaps[i + j]->a_hang(), overlaps2[j]->a_hang());
            ASSERT_EQ(overlaps[i + j]->b(), overlaps2[j]->b());
            ASSERT_EQ(overlaps[i + j]->b_hang(), overlaps2[j]->b_hang());
            ASSERT_EQ(overlaps[i + j]->is_innie(), overlaps2[j]->is_innie());
            ASSERT_EQ(overlaps[i + j]->is_dovetail(), overlaps2[j]->is_dovetail());
            ASSERT_EQ(overlaps[i + j]->a_lo(), overlaps2[j]->a_lo());
            ASSERT_EQ(overlaps[i + j]->a_hi(), overlaps2[j]->a_hi());
            ASSERT_EQ(overlaps[i + j]->b_lo(), overlaps2[j]->b_lo());
            ASSERT_EQ(overlaps[i + j]->b_hi(), overlaps2[j]->b_hi());
            ASSERT_EQ(overlaps[i + j]->err_rate(), overlaps2[j]->err_rate());
            ASSERT_EQ(overlaps[i + j]->orig_err_rate(), overlaps2[j]->orig_err_rate());
        }

        for (const auto& it: overlaps2) delete it;
    }

    ASSERT_EQ(overlaps.size(), total);

    for (const auto& it: overlaps) delete it;
    for (const auto& it: reads) delete it;
    delete depot;
}

TEST(Depot, StoreLoadOverlapsHeavyBy1Overlap) {

    ReadSet reads;
    readFastqReads(reads, "../examples/ERR430949.fastq");

    auto depot = new Depot("depot_dummy");

    OverlapSet overlaps;
    overlapReads(overlaps, reads, 75, 1, "depot_dummy/esa");

    depot->store_overlaps(overlaps);

    OverlapSet overlaps2;

    uint32_t n = 1;
    uint32_t total = 0;

    for (uint32_t i = 0; i < overlaps.size(); i += n) {

        OverlapSet overlaps2;
        depot->load_overlaps(overlaps2, i, n, reads);

        total += overlaps2.size();

        for (uint32_t j = 0; j < overlaps2.size(); ++j) {
            ASSERT_EQ(overlaps[i + j]->a(), overlaps2[j]->a());
            ASSERT_EQ(overlaps[i + j]->a_hang(), overlaps2[j]->a_hang());
            ASSERT_EQ(overlaps[i + j]->b(), overlaps2[j]->b());
            ASSERT_EQ(overlaps[i + j]->b_hang(), overlaps2[j]->b_hang());
            ASSERT_EQ(overlaps[i + j]->is_innie(), overlaps2[j]->is_innie());
            ASSERT_EQ(overlaps[i + j]->is_dovetail(), overlaps2[j]->is_dovetail());
            ASSERT_EQ(overlaps[i + j]->a_lo(), overlaps2[j]->a_lo());
            ASSERT_EQ(overlaps[i + j]->a_hi(), overlaps2[j]->a_hi());
            ASSERT_EQ(overlaps[i + j]->b_lo(), overlaps2[j]->b_lo());
            ASSERT_EQ(overlaps[i + j]->b_hi(), overlaps2[j]->b_hi());
            ASSERT_EQ(overlaps[i + j]->err_rate(), overlaps2[j]->err_rate());
            ASSERT_EQ(overlaps[i + j]->orig_err_rate(), overlaps2[j]->orig_err_rate());
        }

        for (const auto& it: overlaps2) delete it;
    }

    ASSERT_EQ(overlaps.size(), total);

    for (const auto& it: overlaps) delete it;
    for (const auto& it: reads) delete it;
    delete depot;
}
