
#include "AfgOverlap.hpp"
#include "CommonHeaders.hpp"
#include "Overlap.hpp"
#include "ReadIndex.hpp"

static void overlapReadsPart(std::vector<DovetailOverlap*>& dst, const std::vector<Read*>& reads,
    int rk, int minOverlapLen, int threadLen, const char* path, const char* ext);

static bool compareOverlaps(const DovetailOverlap* left, const DovetailOverlap* right);

static void threadOverlapReads(std::vector<DovetailOverlap*>& dst, const std::vector<Read*>& reads,
    int rk, int minOverlapLen, const ReadIndex* rindex, int start, int end);

static void pickMatches(std::vector<DovetailOverlap*>& dst, int i, std::vector<std::pair<int, int>>& matches,
    int type, const std::vector<Read*>& reads);


static void threadFilterTransitive(std::vector<bool>& dst, const std::vector<DovetailOverlap*>& overlaps,
    const std::map<int, std::vector<std::pair<int, DovetailOverlap*>>>& edges, size_t start, size_t end) {

    for (size_t i = start; i < end; ++i) {

        const DovetailOverlap* overlap = overlaps[i];

        const auto& v1 = edges.at(overlap->a());
        const auto& v2 = edges.at(overlap->b());

        auto it1 = v1.begin();
        auto it2 = v2.begin();

        bool transitive = false;

        while (!transitive && it1 != v1.end() && it2 != v2.end()) {

            if (it1->first == overlap->a() || it1->first == overlap->b()) {
                ++it1;
                continue;
            }

            if (it2->first == overlap->a() || it2->first == overlap->b()) {
                ++it2;
                continue;
            }

            if (it1->first == it2->first) {

                auto iStart = it1;
                auto iEnd = iStart;
                for (auto i = iStart; i != v1.end() && i->first == iStart->first; ++i) {
                  iEnd++;
                }

                auto jStart = it2;
                auto jEnd = jStart;
                for (auto j = jStart; j != v2.end() && j->first == jStart->first; ++j) {
                  jEnd++;
                }

                for (auto i = iStart; i != iEnd; ++i) {
                  for (auto j = jStart; j != jEnd; ++j) {
                    if (overlap->is_transitive(i->second, j->second)) {
                      transitive = true;
                      break;
                    }
                  }
                }

                it1 = iEnd;
                it2 = jEnd;
            } else if (it1->first < it2->first) {
                ++it1;
            } else {
                ++it2;
            }
        }

        dst[i] = transitive;
    }
}

void filterContainedOverlaps(std::vector<DovetailOverlap*>& dst, const std::vector<DovetailOverlap*>& overlaps,
    std::vector<Read*>& reads, bool view) {

    Timer timer;
    timer.start();

    int maxId = 0;

    for (const auto& overlap : overlaps) {
        maxId = std::max(std::max(overlap->a(), overlap->b()), maxId);
    }

    std::vector<bool> contained(maxId + 1, false);

    for (const auto& overlap : overlaps) {
        // A    --------->
        // B -----------------
        const auto a = overlap->a();
        const auto b = overlap->b();
        if (overlap->is_using_prefix(a) && overlap->is_using_suffix(a)) {
            // readA is contained
            contained[a] = true;
            debug("ISCONT %d\n", a);

            reads[b]->addCoverage(reads[a]->getLength() / (double) reads[b]->getLength());

            continue;
        }

        // A ---------------->
        // B      ------
        if (overlap->is_using_prefix(b) && overlap->is_using_suffix(b)) {
            // readB is contained
            contained[b] = true;
            debug("ISCONT %d\n", b);

            reads[a]->addCoverage(reads[b]->getLength() / (double) reads[a]->getLength());
        }
    }

    for (const auto& overlap : overlaps) {
        if (contained[overlap->a()] || contained[overlap->b()]) {
          debug("SKIPCONT %d %d\n", overlap->a(), overlap->b());
          continue;
        }

        dst.push_back(view ? overlap : overlap->clone());
    }

    fprintf(stderr, "[Overlap][filter contained]: reduction percentage = %.2lf%%\n",
        (1 - (dst.size() / (double) overlaps.size())) * 100);

    timer.stop();
    timer.print("Overlap", "filter contained");
}

void filterTransitiveOverlaps(std::vector<DovetailOverlap*>& dst, const std::vector<DovetailOverlap*>& overlaps,
    int threadLen, bool view) {

    Timer timer;
    timer.start();

    std::map<int, std::vector<std::pair<int, DovetailOverlap*>>> edges;

    for (const auto& overlap : overlaps) {
        edges[overlap->a()].emplace_back(overlap->b(), overlap);
        edges[overlap->b()].emplace_back(overlap->a(), overlap);
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
        } else {
            debug("SKIPTRAN %d %d\n", overlaps[i]->a(), overlaps[i]->b());
        }
    }

    fprintf(stderr, "[Overlap][filter transitive]: reduction percentage = %.2lf%%\n",
        (1 - (dst.size() / (double) overlaps.size())) * 100);

    timer.stop();
    timer.print("Overlap", "filter transitive");
}

void overlapReads(std::vector<DovetailOverlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path) {

    Timer timer;
    timer.start();

    std::vector<DovetailOverlap*> overlaps;

    overlapReadsPart(overlaps, reads, 0, minOverlapLen, threadLen, path, ".nra");
    overlapReadsPart(overlaps, reads, 1, minOverlapLen, threadLen, path, ".rra");

    fprintf(stderr, "[Overlap][overlaps]: number of overlaps = %zu\n", overlaps.size());

    std::sort(overlaps.begin(), overlaps.end(), compareOverlaps);

    std::vector<DovetailOverlap*> duplicates;

    dst.reserve(overlaps.size());

    for (size_t i = 0; i < overlaps.size(); ++i) {

        if (i > 0 && overlaps[i]->a() == overlaps[i - 1]->a() &&
            overlaps[i]->b() == overlaps[i - 1]->b()) {

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

static void overlapReadsPart(std::vector<DovetailOverlap*>& dst, const std::vector<Read*>& reads,
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

    std::vector<std::vector<DovetailOverlap*>> overlaps(threadLen);

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
        std::vector<DovetailOverlap*>().swap(overlaps[i]);
    }

    delete rindex;
}

static void threadOverlapReads(std::vector<DovetailOverlap*>& dst, const std::vector<Read*>& reads,
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

static bool compareOverlaps(const DovetailOverlap* left, const DovetailOverlap* right) {
    if (left->a() != right->a()) return left->a() < right->a();
    if (left->b() != right->b()) return left->b() < right->b();

    return left->length() > right->length();
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
static void pickMatches(std::vector<DovetailOverlap*>& dst, int i, std::vector<std::pair<int, int>>& matches,
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

