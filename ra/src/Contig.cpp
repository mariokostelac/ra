
#include "Contig.hpp"

// Contig

Contig::Contig(const StringGraphWalk* walk) {

    ASSERT(walk, "Contig", "invalid walk");

    // isReversed returns if read with given id is reversed in edge
    auto isReversed = [](const Edge* edge, uint32_t id) -> int {
        if (edge->getOverlap()->a() == id) return 0; // due to possible overlap types
        if (!edge->getOverlap()->is_innie()) return 0;
        return 1;
    };

    const Vertex* start = walk->getStart();
    const auto& edges = walk->getEdges();

    if (edges.size() == 0) {
      parts_.emplace_back(start->getId(), 0, start->getLength(), 0);
      return;
    }

    int firstReversed = isReversed(edges.front(), start->getId());
    uint32_t offset = 0, length = 0;

    // 0 -> reversed complement
    // 1 -> normal direction
    int prefixGoesFirst = edges.front()->getOverlap()->is_using_suffix(start->getId()) ^ firstReversed;

    uint32_t lo = prefixGoesFirst ? 0 : start->getLength();
    uint32_t hi = prefixGoesFirst ? start->getLength() : 0;

    parts_.emplace_back(start->getId(), lo, hi, offset);
    length += start->getLength();

    int prevReversed = firstReversed;

    for (const auto& edge : edges) {

      const Vertex* a = edge->getSrc();
      const Vertex* b = edge->getDst();

      int aReversed = isReversed(edge, a->getId());
      bool invert = aReversed == prevReversed ? false : true;

      int bReversed = isReversed(edge, b->getId()) ^ invert;

      offset = length - edge->getOverlap()->length(a->getId());

      lo = bReversed ? b->getLength() : 0;
      hi = bReversed ? 0 : b->getLength();

      parts_.emplace_back(b->getId(), lo, hi, offset);
      length += edge->getOverlap()->hanging_length(b->getId());

      prevReversed = bReversed;
    }
}
