/*
* Overlap.cpp
*
* Created on: May 14, 2015
*     Author: rvaser
*/

#include "ReadIndex.hpp"
#include "Overlap.hpp"

const double EPSILON = 0.15;
const double ALPHA = 3;

static inline bool doubleEq(double x, double y, double eps) {
    return y <= x + eps && x <= y + eps;
}

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
            dst.push_back(new Overlap(i, matches[j].first, matches[j].second,
                -1 * aHang, -1 * bHang, type != 0));

        } else {
            dst.push_back(new Overlap(matches[j].first, i, matches[j].second,
                aHang, bHang, type != 0));
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
            // normal x normal
            rindex->readPrefixSuffixMatches(matches, reads[i], 0, minOverlapLen);
            pickMatches(dst, i, matches, 0, reads);
        }

        // normal x reverse complement | reverse complement x normal
        rindex->readPrefixSuffixMatches(matches, reads[i], rk == 0, minOverlapLen);
        pickMatches(dst, i, matches, rk == 0 ? 2 : 1, reads);
    }
}

static void threadFilterTransitive(std::vector<bool>& dst, const std::vector<Overlap*>& overlaps,
    const std::map<int, std::vector<std::pair<int, Overlap*>>>& edges, size_t start, size_t end) {

    for (size_t i = start; i < end; ++i) {

        const Overlap* overlap = overlaps[i];

        const auto& v1 = edges.at(overlap->getA());
        const auto& v2 = edges.at(overlap->getB());

        auto it1 = v1.begin();
        auto it2 = v2.begin();

        bool transitive = false;

        while (!transitive && it1 != v1.end() && it2 != v2.end()) {

            if (it1->first == overlap->getA() || it1->first == overlap->getB()) {
                ++it1;
                continue;
            }

            if (it2->first == overlap->getA() || it2->first == overlap->getB()) {
                ++it2;
                continue;
            }

            if (it1->first == it2->first) {

                if (overlap->isTransitive(it1->second, it2->second)) {
                    transitive = true;
                }

                ++it1;
                ++it2;

            } else if (it1->first < it2->first) {
                ++it1;

            } else {
                ++it2;
            }
        }

        dst[i] = transitive;
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

Overlap::Overlap(int a, int b, int length, int aHang, int bHang, bool innie) :
    a_(a), b_(b), length_(length), aHang_(aHang), bHang_(bHang), innie_(innie) {
}

bool Overlap::isUsingPrefix(int readId) const {

    if (readId == a_) {
        if (aHang_ <= 0) return true;

    } else if (readId == b_) {
        if (innie_ == false && aHang_ >= 0) return true;
        if (innie_ == true && bHang_ <= 0) return true;
    }

    return false;
}

bool Overlap::isUsingSuffix(int readId) const {

    if (readId == a_) {
        if (bHang_ >= 0) return true;

    } else if (readId == b_) {
        if (innie_ == false && bHang_ <= 0) return true;
        if (innie_ == true && aHang_ >= 0) return true;
    }

    return false;
}

bool Overlap::isTransitive(const Overlap* o2, const Overlap* o3) const {

    auto o1 = this;

    int a = o1->getA();
    int b = o1->getB();
    int c = o2->getA() != a ? o2->getA() : o2->getB();

    if (o2->isUsingSuffix(c) == o3->isUsingSuffix(c)) return false;
    if (o1->isUsingSuffix(a) != o2->isUsingSuffix(a)) return false;
    if (o1->isUsingSuffix(b) != o3->isUsingSuffix(b)) return false;

    if (!doubleEq(
            o2->hang(a) + o3->hang(c),
            o1->hang(a),
            EPSILON * o1->getLength() + ALPHA)) {
        return false;
    }

    if (!doubleEq(
            o2->hang(c) + o3->hang(b),
            o1->hang(b),
            EPSILON * o1->getLength() + ALPHA)) {
        return false;
    }

    return true;
}

int Overlap::hang(int readId) const {

    if (readId == a_) return aHang_;
    if (readId == b_) return bHang_;

    ASSERT(false, "Overlap", "wrong read id");
}

Overlap* Overlap::clone() const {
    return new Overlap(a_, b_, length_, aHang_, bHang_, innie_);
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

void filterContainedOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps,
    std::vector<Read*>& reads, bool view) {

    Timer timer;
    timer.start();

    int maxId = 0;

    for (const auto& overlap : overlaps) {
        maxId = std::max(std::max(overlap->getA(), overlap->getB()), maxId);
    }

    std::vector<bool> contained(maxId + 1, false);

    for (const auto& overlap : overlaps) {
        // A    --------->
        // B -----------------
        if (overlap->getAHang() <= 0 && overlap->getBHang() >= 0) {
            // readA is contained
            contained[overlap->getA()] = true;

            reads[overlap->getB()]->addCoverage(reads[overlap->getA()]->getLength() /
                (double) reads[overlap->getB()]->getLength());

            continue;
        }

        // A ---------------->
        // B      ------
        if (overlap->getAHang() >= 0 && overlap->getBHang() <= 0) {
            // readB is contained
            contained[overlap->getB()] = true;

            reads[overlap->getA()]->addCoverage(reads[overlap->getB()]->getLength() /
                (double) reads[overlap->getA()]->getLength());
        }
    }

    for (const auto& overlap : overlaps) {
        if (contained[overlap->getA()] || contained[overlap->getB()]) continue;

        dst.push_back(view ? overlap : overlap->clone());
    }

    fprintf(stderr, "[Overlap][filter contained]: reduction percentage = %.2lf%%\n",
        (1 - (dst.size() / (double) overlaps.size())) * 100);

    timer.stop();
    timer.print("Overlap", "filter contained");
}

void filterTransitiveOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps,
    int threadLen, bool view) {

    Timer timer;
    timer.start();

    std::map<int, std::vector<std::pair<int, Overlap*>>> edges;

    for (const auto& overlap : overlaps) {
        edges[overlap->getA()].emplace_back(overlap->getB(), overlap);
        edges[overlap->getB()].emplace_back(overlap->getA(), overlap);
    }

    for (auto& edge : edges) {
        std::sort(edge.second.begin(), edge.second.end());
    }

    size_t taskLen = std::ceil((double) overlaps.size() / threadLen);
    size_t start = 0;
    size_t end = taskLen;

    std::vector<std::thread> threads;

    std::vector<bool> transitive(overlaps.size(), false);

    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(threadFilterTransitive, std::ref(transitive), std::ref(overlaps),
            std::ref(edges), start, end);

        start = end;
        end = std::min(end + taskLen, overlaps.size());
    }

    for (auto& it : threads) {
        it.join();
    }

    for (size_t i = 0; i < overlaps.size(); ++i) {

        if (!transitive[i]) {
            dst.push_back(view ? overlaps[i] : overlaps[i]->clone());
        }
    }

    fprintf(stderr, "[Overlap][filter transitive]: reduction percentage = %.2lf%%\n",
        (1 - (dst.size() / (double) overlaps.size())) * 100);

    timer.stop();
    timer.print("Overlap", "filter transitive");
}
