/*
* AfgOverlap.cpp
*
* Created on: May 14, 2015
*     Author: rvaser
*/

#include "ReadIndex.hpp"
#include "AfgOverlap.hpp"

static bool compareOverlaps(const Overlap* left, const Overlap* right) {
    if (left->getA() != right->getA()) return left->getA() < right->getA();
    if (left->getB() != right->getB()) return left->getB() < right->getB();

    return left->getLength() > right->getLength();
}

static bool compareMatches(const std::pair<int, int>& left, const std::pair<int, int>& right) {
    if (left.first != right.first) return left.first < right.first;
    return left.second > right.second;
}

// pick all matches of type
// types:
//     0 - id different from i (normal x normal)
//     1 - id greater than i (normal x reverse complement)
//     2 - id less than i (reverse complement x normal)
static void pickMatches(std::vector<Overlap*>& dst, int i, std::vector<std::pair<int, int>>& matches,
    int type, const std::vector<Read*>& reads) {

    if (matches.size() == 0) return;

    std::sort(matches.begin(), matches.end(), compareMatches);

    for (int j = 0; j < (int) matches.size(); ++j) {

        if (j > 0 && matches[j].first == matches[j - 1].first) continue;

        switch (type) {
            case 0:
                if (matches[j].first == i) continue;
                break;
            case 1:
                if (matches[j].first <= i) continue;
                break;
            case 2:
            default:
                if (matches[j].first >= i) continue;
                break;
        }

        int aHang = reads[matches[j].first]->getLength() - matches[j].second;
        int bHang = reads[i]->getLength() - matches[j].second;

        if (i < matches[j].first) {
            dst.push_back(new AfgOverlap(i, matches[j].first, matches[j].second,
                -1 * aHang, -1 * bHang, type != 0));

        } else {
            dst.push_back(new AfgOverlap(matches[j].first, i, matches[j].second,
                aHang, bHang, type != 0));
        }
    }

    matches.clear();
}

static void threadOverlapReads(std::vector<Overlap*>& dst, const std::vector<Read*>& reads,
    int rk, int minOverlapLen, const ReadIndex* rindex, int start, int end) {

    std::vector<std::pair<int, int>> matches;

    for (int i = start; i < end; ++i) {

        if (rk == 0) {
            // normal x normal
            rindex->readPrefixSuffixMatches(matches, reads[i], 0, minOverlapLen);
            pickMatches(dst, i, matches, 0, reads);
        }

        // normal x reverse complement | reverse complement x normal
        rindex->readPrefixSuffixMatches(matches, reads[i], rk == 0, minOverlapLen);
        pickMatches(dst, i, matches, rk == 0 ? 2 : 1, reads);
    }
}

static void overlapReadsPart(std::vector<Overlap*>& dst, const std::vector<Read*>& reads,
    int rk, int minOverlapLen, int threadLen, const char* path, const char* ext) {

    std::string cache = path;
    cache += ext;

    ReadIndex* rindex = ReadIndex::load(cache.c_str());

    if (rindex == nullptr) {
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

AfgOverlap::AfgOverlap(int a, int b, int length, int aHang, int bHang, bool innie) :
    Overlap(a, b), length_(length), aHang_(aHang), bHang_(bHang), innie_(innie) {
}

bool AfgOverlap::isUsingPrefix(int readId) const {

    if (readId == getA()) {
        if (aHang_ <= 0) return true;

    } else if (readId == getB()) {
        if (innie_ == false && aHang_ >= 0) return true;
        if (innie_ == true && bHang_ <= 0) return true;
    }

    return false;
}

bool AfgOverlap::isUsingSuffix(int readId) const {

    if (readId == getA()) {
        if (bHang_ >= 0) return true;

    } else if (readId == getB()) {
        if (innie_ == false && bHang_ <= 0) return true;
        if (innie_ == true && aHang_ >= 0) return true;
    }

    return false;
}


uint AfgOverlap::hangingLength(int readId) const {

    if (readId == getA()) return abs(aHang_);
    if (readId == getB()) return abs(bHang_);

    ASSERT(false, "Overlap", "wrong read id");
}

int AfgOverlap::getLength() const {
    return (getLengthA() + getLengthB())/2;
}

int AfgOverlap::getLength(int read_id) const {
    if (read_id == getA()) {
        return getLengthA();
    } else if (read_id == getB()) {
        return getLengthB();
    }

    assert(false);
}

int AfgOverlap::getLengthA() const {
    ASSERT(getReadA() != nullptr, "Overlap", "Read* a is nullptr");

    int len = getReadA()->getSequence().length();
    if (aHang_ > 0) {
      len -= aHang_;
    }
    if (bHang_ < 0) {
      len -= abs(bHang_);
    }

    return len;
}

int AfgOverlap::getLengthB() const {
    ASSERT(getReadB() != nullptr, "Overlap", "Read* b is nullptr");

    int len = getReadB()->getSequence().length();
    if (aHang_ < 0) {
      len -= abs(aHang_);
    }
    if (bHang_ > 0) {
      len -= bHang_;
    }

    return len;
}

Overlap* AfgOverlap::clone() const {
    return new AfgOverlap(getA(), getB(), length_, aHang_, bHang_, innie_);
}

void AfgOverlap::print(std::ostream& o) const {
  o << "{OVL" << std::endl;
  o << "adj:" << (innie_ ? 'I' : 'N') << std::endl;
  o << "rds:" << getA() << "," << getB() << std::endl;
  o << "ahg:" << aHang_ << std::endl;
  o << "bhg:" << bHang_ << std::endl;
  o << "scr:" << getScore() << std::endl;
  o << "}" << std::endl;
}

void overlapReads(std::vector<Overlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path) {

    Timer timer;
    timer.start();

    std::vector<Overlap*> overlaps;

    overlapReadsPart(overlaps, reads, 0, minOverlapLen, threadLen, path, ".nra");
    overlapReadsPart(overlaps, reads, 1, minOverlapLen, threadLen, path, ".rra");

    fprintf(stderr, "[Overlap][overlaps]: number of overlaps = %zu\n", overlaps.size());

    std::sort(overlaps.begin(), overlaps.end(), compareOverlaps);

    std::vector<Overlap*> duplicates;

    dst.reserve(overlaps.size());

    for (size_t i = 0; i < overlaps.size(); ++i) {

        if (i > 0 && overlaps[i]->getA() == overlaps[i - 1]->getA() &&
            overlaps[i]->getB() == overlaps[i - 1]->getB()) {

            duplicates.emplace_back(overlaps[i]);
            continue;
        }

        dst.emplace_back(overlaps[i]);
    }

    for (const auto& duplicate : duplicates) delete duplicate;

    fprintf(stderr, "[Overlap][overlaps]: number of unique overlaps = %zu\n", dst.size());

    timer.stop();
    timer.print("Overlap", "overlaps");
}

