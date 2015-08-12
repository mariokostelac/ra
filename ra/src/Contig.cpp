
#include "Contig.hpp"

// Contig

Contig::Contig(const StringGraphWalk* walk) {

    ASSERT(walk, "Contig", "invalid walk");

    auto getType = [](const Edge* edge, int id) -> int {
        if (edge->getOverlap()->getA() == id) return 0; // due to possible overlap types
        if (!edge->getOverlap()->isInnie()) return 0;
        return 1;
    };

    const Vertex* start = walk->getStart();
    const auto& edges = walk->getEdges();

    int startType = getType(edges.front(), start->getId());
    int offset = 0;

    int direction = edges.front()->getOverlap()->isUsingSuffix(start->getId()) ^ startType;

    int lo = direction ? 0 : start->getLength();
    int hi = direction ? start->getLength() : 0;

    parts_.emplace_back(start->getId(), startType, offset, lo, hi);

    int prevType = startType;

    for (const auto& edge : edges) {

        const Vertex* a = edge->getSrc();
        const Vertex* b = edge->getDst();

        int typeA = getType(edge, a->getId());
        bool invert = typeA == prevType ? false : true;

        int typeB = getType(edge, b->getId()) ^ invert;

        offset += a->getLength() - edge->getOverlap()->getLength(a->getId());

        lo = direction ? 0 : b->getLength();
        hi = direction ? b->getLength() : 0;

        parts_.emplace_back(b->getId(), typeB, offset, lo, hi);

        prevType = typeB;
    }
}

