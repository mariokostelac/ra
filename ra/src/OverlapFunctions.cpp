#include "ReadIndex.hpp"
#include "EditDistance.hpp"
#include "OverlapFunctions.hpp"

static void overlapReadsPart(std::vector<Overlap*>& dst, const std::vector<Read*>& reads,
    int rk, int minOverlapLen, int threadLen, const char* path, const char* ext);

static bool compareOverlaps(const Overlap* left, const Overlap* right);

static void threadOverlapReads(std::vector<Overlap*>& dst, const std::vector<Read*>& reads,
    int rk, int minOverlapLen, const ReadIndex* rindex, int start, int end);

static void pickMatches(std::vector<Overlap*>& dst, int i, std::vector<std::pair<int, int>>& matches,
    int type, const std::vector<Read*>& reads);

void filterContainedOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps,
    std::vector<Read*>& reads, bool view) {

    Timer timer;
    timer.start();

    uint32_t maxId = 0;

    for (const auto& overlap : overlaps) {
        maxId = std::max(std::max(overlap->a(), overlap->b()), maxId);
    }

    std::vector<bool> contained(maxId + 1, false);

    for (const auto& o : overlaps) {
        // A    --------->
        // B -----------------
        const auto a = o->a();
        const auto b = o->b();

        const auto a_lo = o->a_lo(), a_hi = o->a_hi();
        const auto a_len = o->read_a()->length();

        const auto b_lo = o->b_lo(), b_hi = o->b_hi();
        const auto b_len = o->read_b()->length();

        const auto hangs = calculateForcedHangs(a_lo, a_hi, a_len, b_lo, b_hi, b_len);

        if (hangs.first <= 0 && hangs.second >= 0) {
            // read_a is contained
            contained[a] = true;
            debug("ISCONT %d in %d,%d\n", a, a, b);

            continue;
        }

        // A ---------------->
        // B      ------
        if (hangs.first >= 0 && hangs.second <= 0) {
            // read_b is contained
            contained[b] = true;
            debug("ISCONT %d in %d,%d\n", b, a, b);
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

void filterTransitiveOverlaps(std::vector<Overlap*>& dst, const OverlapSet& overlaps,
    int threadLen, bool view) {

    Timer timer;
    timer.start();

    std::map<uint32_t, std::vector<std::pair<uint32_t, Overlap*>>> edges;

    for (const auto& overlap : overlaps) {
        edges[overlap->a()].emplace_back(overlap->b(), overlap);
        edges[overlap->b()].emplace_back(overlap->a(), overlap);
    }

    for (auto& edge : edges) {
        std::sort(edge.second.begin(), edge.second.end());
    }

    std::vector<bool> transitive(overlaps.size(), false);

    for (size_t i = 0; i < overlaps.size(); ++i) {

        const Overlap* overlap = overlaps[i];

        const auto& v1 = edges.at(overlap->a());
        const auto& v2 = edges.at(overlap->b());

        auto it1 = v1.begin();
        auto it2 = v2.begin();

        bool is_tran = false;

        while (!is_tran && it1 != v1.end() && it2 != v2.end()) {

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
                      is_tran = true;
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

        transitive[i] = is_tran;
    }

    for (size_t i = 0; i < overlaps.size(); ++i) {
        if (!transitive[i]) {
            dst.push_back(view ? overlaps[i] : overlaps[i]->clone());
        } else {
            debug("SKIPTRAN %d %d\n", overlaps[i]->a(), overlaps[i]->b());
        }
    }

#ifdef DEBUG
    fprintf(stderr, "[Overlap][filter transitive]: reduction percentage = %.2lf%%\n",
        (1 - (dst.size() / (double) overlaps.size())) * 100);
#endif

    timer.stop();
    timer.print("Overlap", "filter transitive");
}

void overlapReads(std::vector<Overlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path) {

    Timer timer;
    timer.start();

    std::vector<Overlap*> overlaps;

    overlapReadsPart(overlaps, reads, 0, minOverlapLen, threadLen, path, ".nra");
    overlapReadsPart(overlaps, reads, 1, minOverlapLen, threadLen, path, ".rra");

#ifdef DEBUG
    fprintf(stderr, "[Overlap][overlaps]: number of overlaps = %zu\n", overlaps.size());
#endif

    std::sort(overlaps.begin(), overlaps.end(), compareOverlaps);

    std::vector<Overlap*> duplicates;

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

#ifdef DEBUG
    fprintf(stderr, "[Overlap][overlaps]: number of unique overlaps = %zu\n", dst.size());
#endif

    timer.stop();
    timer.print("Overlap", "overlaps");
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

static bool compareOverlaps(const Overlap* left, const Overlap* right) {
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

        int aHang = reads[matches[j].first]->length() - matches[j].second;
        int bHang = reads[i]->length() - matches[j].second;

        if (i < matches[j].first) {
            dst.push_back(new Overlap(reads[i], -1 * aHang, reads[matches[j].first],
                -1 * bHang, type != 0));

        } else {
            dst.push_back(new Overlap(reads[matches[j].first], aHang, reads[i],
                bHang, type != 0));
        }
    }

    matches.clear();
}

std::pair<int, int> calculateForcedHangs(uint32_t a_lo, uint32_t a_hi, uint32_t a_len,
    uint32_t b_lo, uint32_t b_hi, uint32_t b_len) {

    std::pair<int, int> hangs;

    // -----|------|---->
    //  ah -|------|---->
    //
    // -ah -|------|---->
    // -----|------|---->
    hangs.first = a_lo - b_lo;

    //     -|------|-> bh
    // -----|------|------>
    //
    // -----|------|------>
    //     -|------|-> -bh
    int b_after = b_len - b_hi;
    int a_after = a_len - a_hi;
    hangs.second = b_after - a_after;

    return hangs;
}

Overlap* forcedDovetailOverlap(const Overlap* o, bool calc_error_rates) {

    if (o->is_dovetail()) {
        return o->clone();
    }

    auto a_part = o->extract_overlapped_part(o->a());
    auto b_part = o->extract_overlapped_part(o->b());

    double orig_err_rate = editDistance(a_part, b_part) / (double) o->length();

    const auto hangs = calculateForcedHangs(
        o->a_lo(), o->a_hi(), o->read_a()->length(),
        o->b_lo(), o->b_hi(), o->read_b()->length()
    );

    Overlap tmp(o->read_a(), hangs.first, o->read_b(), hangs.second, o->is_innie());

    a_part = tmp.extract_overlapped_part(tmp.a());
    b_part = tmp.extract_overlapped_part(tmp.b());

    double err_rate = editDistance(a_part, b_part) / (double) tmp.length();

    auto res = new Overlap(tmp.read_a(), tmp.a_hang(), tmp.read_b(), tmp.b_hang(),
        tmp.is_innie(), err_rate, orig_err_rate);

    return res;
}
