
#include "Contig.hpp"

// Contig

Contig::Contig(const StringGraphWalk* walk) {

    ASSERT(walk, "Contig", "invalid walk");

    // isReversed returns if read with given id is reversed in edge
    auto isReversed = [](const Edge* edge, int id) -> int {
        if (edge->getOverlap()->getA() == id) return 0; // due to possible overlap types
        if (!edge->getOverlap()->isInnie()) return 0;
        return 1;
    };

    const Vertex* start = walk->getStart();
    const auto& edges = walk->getEdges();

    if (edges.size() == 0) {
      parts_.emplace_back(start->getId(), 0, start->getLength(), 0);
    } else {
      int firstReversed = isReversed(edges.front(), start->getId());
      int offset = 0;

      // 0 -> reversed complement
      // 1 -> normal direction
      int prefixGoesFirst = edges.front()->getOverlap()->isUsingSuffix(start->getId()) ^ firstReversed;

      int lo = prefixGoesFirst ? 0 : start->getLength();
      int hi = prefixGoesFirst ? start->getLength() : 0;

      parts_.emplace_back(start->getId(), lo, hi, offset);

      int prevReversed = firstReversed;

      for (const auto& edge : edges) {

        const Vertex* a = edge->getSrc();
        const Vertex* b = edge->getDst();

        int aReversed = isReversed(edge, a->getId());
        bool invert = aReversed == prevReversed ? false : true;

        int bReversed = isReversed(edge, b->getId()) ^ invert;

        offset += a->getLength() - edge->getOverlap()->getLength(a->getId());

        lo = bReversed ? b->getLength() : 0;
        hi = bReversed ? 0 : b->getLength();

        parts_.emplace_back(b->getId(), lo, hi, offset);

        prevReversed = bReversed;
      }
    }
}
