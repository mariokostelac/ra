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
    int i, const std::vector<Read*>& reads) {

    if (matches.size() == 0) return;

    std::sort(matches.begin(), matches.end(), compareMatches);

    if (matches[0].first != i) {
        dst.push_back(new Overlap(
            reads[i],
            reads[matches[0].first],
            -1 * (reads[matches[0].first]->getLength() - matches[0].second),
            -1 * (reads[i]->getLength() - matches[0].second),
            false
        ));
    }

    for (int j = 1; j < (int) matches.size(); ++j) {

        if (matches[j].first == matches[j - 1].first || matches[j].first == i) continue;

        dst.push_back(new Overlap(
            reads[i],
            reads[matches[j].first],
            -1 * (reads[matches[j].first]->getLength() - matches[j].second),
            -1 * (reads[i]->getLength() - matches[j].second),
            false
        ));
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

        if (rk == 1) { // normal x rk
            dst.push_back(new Overlap(
                reads[i],
                reads[matches[0].first],
                -1 * (reads[matches[0].first]->getLength() - matches[0].second),
                -1 * (reads[i]->getLength() - matches[0].second),
                true
            ));

        } else { // rk x normal
            dst.push_back(new Overlap(
                reads[matches[0].first],
                reads[i],
                reads[i]->getLength() - matches[0].second,
                reads[matches[0].first]->getLength() - matches[0].second,
                true
            ));
        }
    }

    for (int j = 1; j < (int) matches.size(); ++j) {

        if (matches[j].first == matches[j - 1].first || matches[j].first <= i) continue;

        if (rk == 1) { // normal x rk
            dst.push_back(new Overlap(
                reads[i],
                reads[matches[j].first],
                -1 * (reads[matches[j].first]->getLength() - matches[j].second),
                -1 * (reads[i]->getLength() - matches[j].second),
                true
            ));

        } else { // rk x normal
            dst.push_back(new Overlap(
                reads[matches[j].first],
                reads[i],
                reads[i]->getLength() - matches[j].second,
                reads[matches[j].first]->getLength() - matches[j].second,
                true
            ));
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
            pickMatches(dst, matches, i, reads);
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

Overlap::Overlap(const Read* a, const Read* b, int aHang, int bHang, bool innie) :
    a_(a), b_(b), aHang_(aHang), bHang_(bHang), innie_(innie) {
}

Overlap* Overlap::clone() const {

    return new Overlap(a_, b_, aHang_, bHang_, innie_);
}

void Overlap::print() const {

    if (aHang_ < 0) printf("%s", std::string(abs(aHang_), ' ').c_str());
    printf("%s\n", a_->getSequence().c_str());

    if (aHang_ > 0) printf("%s\n", std::string(aHang_, ' ').c_str());
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

    fprintf(stderr, "[Overlap][overlaps]: number of overlaps = %zu\n", dst.size());

    timer.stop();
    timer.print("Overlap", "overlaps");
}

void filterContainedOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps, bool view) {

    Timer timer;
    timer.start();

    std::set<int> contained;

    for (const auto& it : overlaps) {
        // A    --------->
        // B -----------------
        if (it->getAHang() <= 0 && it->getBHang() >= 0) {
            // readA is contained
            contained.insert(it->getReadA()->getId());
            continue;
        }

        // A ---------------->
        // B      ------
        if (it->getAHang() >= 0 && it->getBHang() <= 0) {
            // readB is contained
            contained.insert(it->getReadB()->getId());
        }
    }

    for (const auto& it : overlaps) {
        if (contained.count(it->getReadA()->getId()) > 0) continue;
        if (contained.count(it->getReadB()->getId()) > 0) continue;

        dst.push_back(view ? it : it->clone());
    }

    fprintf(stderr, "[Overlap][filter contained]: %.2lf%%\n",
        (1 - dst.size() / (double) overlaps.size()) * 100);

    timer.stop();
    timer.print("Overlap", "filter contained");
}

void filterTransitiveOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps, bool view) {

    Timer timer;
    timer.start();

    fprintf(stderr, "[Overlap][filter contained]: %.2lf%%\n",
        (1 - dst.size() / (double) overlaps.size()) * 100);

    timer.stop();
    timer.print("Overlap", "filter transitive");
}
