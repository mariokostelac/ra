/*
* Overlap.cpp
*
* Created on: May 14, 2015
*     Author: rvaser
*/

#include "ReadIndex.hpp"
#include "Overlap.hpp"

static void createReverseComplements(std::vector<Read*>& reads, int start, int end) {

    for (int i = start; i < end; ++i) {
        reads[i]->createReverseComplement();
    }
}

static void overlapReads(std::vector<Overlap*>& dst, const std::vector<Read*>& reads,
    int start, int end, int minOverlapLen, int rk, const ReadIndex* rindex) {

    std::vector<int> overlaps;

    for (int i = start; i < end; ++i) {
        rindex->getPrefixSuffixMatches(overlaps, reads[i]->getSequence().c_str(),
            reads[i]->getLength(), minOverlapLen);

        for (int j = 0; j < (int) overlaps.size(); ++j) {

            if (overlaps[j] == 0 || i == j) continue;

            if (i < j) {
                dst.push_back(new Overlap(reads[i], reads[j], overlaps[j],
                    -1 * (reads[i]->getLength() - overlaps[j]),
                    -1 * (reads[j]->getLength() - overlaps[j]),
                    rk == 1));

            } else {
                dst.push_back(new Overlap(reads[j], reads[i], overlaps[j],
                    reads[j]->getLength() - overlaps[j],
                    reads[i]->getLength() - overlaps[j],
                    rk == 1));
            }
        }
    }
}

static void getOverlapsParralel(std::vector<Overlap*>& dst, const std::vector<Read*>& reads,
    int minOverlapLen, int rk, int threadLen, const char* path, const char* ext) {

    ReadIndex* rindex = createReadIndex(reads, rk, path, ext);

    int taskLen = std::ceil((double) reads.size() / threadLen);
    int start = 0;
    int end = taskLen;

    std::vector<std::thread> threads;

    std::vector<std::vector<Overlap*>> overlaps(threadLen);

    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(overlapReads, std::ref(overlaps[i]), std::ref(reads), start, end,
            minOverlapLen, rk, rindex);

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

void getOverlaps(std::vector<Overlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path) {

    Timer timer;
    timer.start();

    int taskLen = std::ceil((double) reads.size() / threadLen);
    int start = 0;
    int end = taskLen;

    std::vector<std::thread> threads;

    // create reverse complements
    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(createReverseComplements, std::ref(reads), start, end);

        start = end;
        end = std::min(end + taskLen, (int) reads.size());
    }

    for (auto& it : threads) {
        it.join();
    }

    std::vector<std::thread>().swap(threads);

    getOverlapsParralel(dst, reads, minOverlapLen, 0, threadLen, path, "nra");
    // getOverlapsParralel(dst, reads, minOverlapLen, 1, threadLen, path, "rra");

    timer.stop();
    timer.print("Overlap", "overlaps");
}
