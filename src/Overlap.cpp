/*
* Overlap.cpp
*
* Created on: May 14, 2015
*     Author: rvaser
*/

#include "ReadIndex.hpp"
#include "Overlap.hpp"

static bool compareMatches(const std::pair<int, int>& left, const std::pair<int, int>& right) {
    return left.first < right.first || (left.first == right.first && left.second > right.second);
}

// pick all matches with id different than i
// normal x normal overlaps
static void pickMatches(std::vector<OverlapPtr>& dst, std::vector<std::pair<int, int>>& matches,
    int i, const std::vector<ReadPtr>& reads, int rk) {

    if (matches.size() == 0) return;

    std::sort(matches.begin(), matches.end(), compareMatches);

    if (matches[0].first != i) {
        dst.emplace_back(std::make_shared<Overlap>(
            i,
            matches[0].first,
            matches[0].second,
            -1 * (reads[matches[0].first]->getLength() - matches[0].second),
            -1 * (reads[i]->getLength() - matches[0].second),
            rk
        ));
    }

    for (int j = 1; j < (int) matches.size(); ++j) {

        if (matches[j].first == matches[j - 1].first || matches[j].first == i) continue;

        dst.emplace_back(std::make_shared<Overlap>(
            i,
            matches[j].first,
            matches[j].second,
            -1 * (reads[matches[j].first]->getLength() - matches[j].second),
            -1 * (reads[i]->getLength() - matches[j].second),
            rk
        ));
    }

    matches.clear();
}

// pick all matches with id Greater Than i
// normal x reverseComplement & reverseComplement x normal overlaps
static void pickMatchesGT(std::vector<OverlapPtr>& dst, std::vector<std::pair<int, int>>& matches,
    int i, const std::vector<ReadPtr>& reads, int rk) {

    if (matches.size() == 0) return;

    std::sort(matches.begin(), matches.end(), compareMatches);

    if (matches[0].first > i) {

        if (rk == 1) {
            dst.emplace_back(std::make_shared<Overlap>(
                i,
                matches[0].first,
                matches[0].second,
                -1 * (reads[matches[0].first]->getLength() - matches[0].second),
                -1 * (reads[i]->getLength() - matches[0].second),
                rk
            ));

        } else {
            dst.emplace_back(std::make_shared<Overlap>(
                matches[0].first,
                i,
                matches[0].second,
                reads[i]->getLength() - matches[0].second,
                reads[matches[0].first]->getLength() - matches[0].second,
                rk
            ));
        }
    }

    for (int j = 1; j < (int) matches.size(); ++j) {

        if (matches[j].first == matches[j - 1].first || matches[j].first <= i) continue;

        if (rk == 1) {
            dst.emplace_back(std::make_shared<Overlap>(
                i,
                matches[j].first,
                matches[j].second,
                -1 * (reads[matches[j].first]->getLength() - matches[j].second),
                -1 * (reads[i]->getLength() - matches[j].second),
                rk
            ));

        } else {
            dst.emplace_back(std::make_shared<Overlap>(
                matches[j].first,
                i,
                matches[j].second,
                reads[i]->getLength() - matches[j].second,
                reads[matches[j].first]->getLength() - matches[j].second,
                rk
            ));
        }
    }

    matches.clear();
}

static void threadCreateReverseComplements(std::vector<ReadPtr>& reads, int start, int end) {

    for (int i = start; i < end; ++i) {
        reads[i]->createReverseComplement();
    }
}

static void threadOverlapReads(std::vector<OverlapPtr>& dst, const std::vector<ReadPtr>& reads,
    int rk, int minOverlapLen, const ReadIndex* rindex, int start, int end) {

    std::vector<std::pair<int, int>> matches;

    for (int i = start; i < end; ++i) {

        if (rk == 0) {
            rindex->readPrefixSuffixMatches(matches, reads[i].get(), 0, minOverlapLen);
            pickMatches(dst, matches, i, reads, rk);
        }

        rindex->readPrefixSuffixMatches(matches, reads[i].get(), rk == 0, minOverlapLen);
        pickMatchesGT(dst, matches, i, reads, rk);
    }
}

static void overlapReadsPart(std::vector<OverlapPtr>& dst, const std::vector<ReadPtr>& reads,
    int rk, int minOverlapLen, int threadLen, const char* path, const char* ext) {

    std::string cache = path;
    cache += ext;

    ReadIndex* rindex = ReadIndex::load(cache.c_str());

    if (rindex == NULL) {
        rindex = new ReadIndex(reads, rk);
        rindex->store(cache.c_str());
    }

    int taskLen = std::ceil((double) reads.size() / threadLen);
    int start = 0;
    int end = taskLen;

    std::vector<std::thread> threads;

    std::vector<std::vector<OverlapPtr>> overlaps(threadLen);

    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(threadOverlapReads, std::ref(overlaps[i]), std::ref(reads), rk,
            minOverlapLen, rindex, start, end);

        start = end;
        end = std::min(end + taskLen, (int) reads.size());
    }

    for (auto& it : threads) {
        it.join();
    }

    // merge overlaps
    for (int i = 0; i < threadLen; ++i) {
        dst.insert(dst.end(), overlaps[i].begin(), overlaps[i].end());
        std::vector<OverlapPtr>().swap(overlaps[i]);
    }

    delete rindex;
}

Overlap::Overlap(int aId, int bId, int length, int aHang, int bHang, bool innie) :
    aId_(aId), bId_(bId), length_(length), aHang_(aHang), bHang_(bHang), innie_(innie) {
}

void overlapReads(std::vector<OverlapPtr>& dst, std::vector<ReadPtr>& reads, int minOverlapLen,
    int threadLen, const char* path) {

    Timer timer;
    timer.start();

    int taskLen = std::ceil((double) reads.size() / threadLen);
    int start = 0;
    int end = taskLen;

    std::vector<std::thread> threads;

    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(threadCreateReverseComplements, std::ref(reads), start, end);

        start = end;
        end = std::min(end + taskLen, (int) reads.size());
    }

    for (auto& it : threads) {
        it.join();
    }

    std::vector<std::thread>().swap(threads);

    overlapReadsPart(dst, reads, 0, minOverlapLen, threadLen, path, ".nra");
    overlapReadsPart(dst, reads, 1, minOverlapLen, threadLen, path, ".rra");

    fprintf(stderr, "[Overlap][overlaps] number of overlaps = %zu\n", dst.size());

    timer.stop();
    timer.print("Overlap", "overlaps");
}
