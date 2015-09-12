
#include "CommonHeaders.hpp"
#include "Overlap.hpp"

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
                    if (overlap->isTransitive(i->second, j->second)) {
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
        const auto a = overlap->getA();
        const auto b = overlap->getB();
        if (overlap->isUsingPrefix(a) && overlap->isUsingSuffix(a)) {
            // readA is contained
            contained[a] = true;
            debug("ISCONT %d\n", a);

            reads[b]->addCoverage(reads[a]->getLength() / (double) reads[b]->getLength());

            continue;
        }

        // A ---------------->
        // B      ------
        if (overlap->isUsingPrefix(b) && overlap->isUsingSuffix(b)) {
            // readB is contained
            contained[b] = true;
            debug("ISCONT %d\n", b);

            reads[a]->addCoverage(reads[b]->getLength() / (double) reads[a]->getLength());
        }
    }

    for (const auto& overlap : overlaps) {
        if (contained[overlap->getA()] || contained[overlap->getB()]) {
          debug("SKIPCONT %d %d\n", overlap->getA(), overlap->getB());
          continue;
        }

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
        } else {
            debug("SKIPTRAN %d %d\n", overlaps[i]->getReadA()->getId(), overlaps[i]->getReadB()->getId());
        }
    }

    fprintf(stderr, "[Overlap][filter transitive]: reduction percentage = %.2lf%%\n",
        (1 - (dst.size() / (double) overlaps.size())) * 100);

    timer.stop();
    timer.print("Overlap", "filter transitive");
}

