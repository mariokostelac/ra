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
static void pickMatches(std::vector<Overlap*>& dst, std::vector<std::pair<int, int>>& matches,
    int i, const std::vector<Read*>& reads, int rk) {

    if (matches.size() == 0) return;

    std::sort(matches.begin(), matches.end(), compareMatches);

    if (matches[0].first != i) {
        dst.push_back(new Overlap(reads[i], reads[matches[0].first], matches[0].second,
            -1 * (reads[matches[0].first]->getLength() - matches[0].second),
            -1 * (reads[i]->getLength() - matches[0].second), rk));
    }

    for (int j = 1; j < (int) matches.size(); ++j) {

        if (matches[j].first == matches[j - 1].first || matches[j].first == i) continue;

        dst.push_back(new Overlap(reads[i], reads[matches[j].first], matches[j].second,
            -1 * (reads[matches[j].first]->getLength() - matches[j].second),
            -1 * (reads[i]->getLength() - matches[j].second), rk));
    }

    matches.clear();
}

// pick all matches with id Greater Than i
// normal x reverseComplement & reverseComplement x normal overlaps
static void pickMatchesGT(std::vector<Overlap*>& dst, std::vector<std::pair<int, int>>& matches,
    int i, const std::vector<Read*>& reads, int rk) {

    if (matches.size() == 0) return;

    std::sort(matches.begin(), matches.end(), compareMatches);

    if (matches[0].first > i) {

        if (rk == 1) {
            dst.push_back(new Overlap(reads[i], reads[matches[0].first], matches[0].second,
                -1 * (reads[matches[0].first]->getLength() - matches[0].second),
                -1 * (reads[i]->getLength() - matches[0].second), rk));
        } else {
            dst.push_back(new Overlap(reads[matches[0].first], reads[i], matches[0].second,
                reads[i]->getLength() - matches[0].second,
                reads[matches[0].first]->getLength() - matches[0].second, rk));
        }
    }

    for (int j = 1; j < (int) matches.size(); ++j) {

        if (matches[j].first == matches[j - 1].first || matches[j].first <= i) continue;

        if (rk == 1) {
            dst.push_back(new Overlap(reads[i], reads[matches[j].first], matches[j].second,
                -1 * (reads[matches[j].first]->getLength() - matches[j].second),
                -1 * (reads[i]->getLength() - matches[j].second), rk));
        } else {
            dst.push_back(new Overlap(reads[matches[j].first], reads[i], matches[j].second,
                reads[i]->getLength() - matches[j].second,
                reads[matches[j].first]->getLength() - matches[j].second, rk));
        }
    }

    matches.clear();
}

static void threadCreateReverseComplements(std::vector<Read*>& reads, int start, int end) {

    for (int i = start; i < end; ++i) {
        reads[i]->createReverseComplement();
    }
}

static void threadOverlapReads(std::vector<Overlap*>& dst, const std::vector<Read*>& reads,
    int rk, int minOverlapLen, const ReadIndex* rindex, int start, int end) {

    std::vector<std::pair<int, int>> matches;

    for (int i = start; i < end; ++i) {

        if (rk == 0) {
            rindex->readPrefixSuffixMatches(matches, reads[i], 0, minOverlapLen);
            //for (const auto& it : matches) printf("%d %d\n", it.first, it.second);
            pickMatches(dst, matches, i, reads, rk);
        }

        rindex->readPrefixSuffixMatches(matches, reads[i], rk == 0, minOverlapLen);
        pickMatchesGT(dst, matches, i, reads, rk);
    }
}

static void overlapReadsPart(std::vector<Overlap*>& dst, const std::vector<Read*>& reads,
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

    std::vector<std::vector<Overlap*>> overlaps(threadLen);

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
        std::vector<Overlap*>().swap(overlaps[i]);
    }

    delete rindex;
}

Overlap::Overlap(const Read* a, const Read* b, int length, int aHang, int bHang, bool innie) :
    a_(a), b_(b), length_(length), aHang_(aHang), bHang_(bHang), innie_(innie) {
}

void Overlap::print() {

    if (aHang_ < 0) printf("%s", std::string(abs(aHang_), ' ').c_str());
    printf("%s\n", a_->getSequence().c_str());

    if (aHang_ > 0) printf("%s", std::string(aHang_, ' ').c_str());
    printf("%s\n\n", (innie_ ? b_->getReverseComplement() : b_->getSequence()).c_str());
}

void overlapReads(std::vector<Overlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
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

    timer.stop();
    timer.print("Overlap", "overlaps");
}
