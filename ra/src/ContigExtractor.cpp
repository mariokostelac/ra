
#include "ContigExtractor.hpp"

namespace std {
  template <>
    struct hash<std::pair<int, bool>> {
      public:
        size_t operator()(std::pair<int, bool> x) const throw() {
          return x.first ^ x.second;
        }
    };
}

Contig* ContigExtractor::extractContig() {

  if (component_->walk_ == nullptr) component_->extractLongestWalk();
  if (component_->walk_ != nullptr) return new Contig(component_->walk_);
  return nullptr;

  //int max_id = 0;

  //for (const auto& v: component_->vertices_) {
      //max_id = std::max(max_id, v->getId());
  //}

  //for (int direction = 0; direction <= 1; ++direction) {

      //for (const auto& vertex : component_->vertices_) {

          //if ((direction == 0 && vertex->getEdgesB().size() == 1 && vertex->getEdgesE().size() == 0) ||
              //(direction == 1 && vertex->getEdgesE().size() == 1 && vertex->getEdgesB().size() == 0)) {

              //std::unordered_map<std::pair<int, bool>, int> cache;
              //std::vector<bool> visited(max_id + 1);

              //for (const auto& v: component_->vertices_) {
                  //fprintf(stdout, "+ %d%c %d\n",
                      //v->getId(),
                      //direction == 0 ? 'B' : 'E',
                      //longestPath(v->getId(), direction, cache, visited)
                  //);
              //}
          //}
      //}
  //}

  //return nullptr;
}

int ContigExtractor::longestPath(int vertexId, bool asBegin,
    std::unordered_map<std::pair<int, bool>, int>& cache,
    std::vector<bool>& visited) {

    const auto& key = std::make_pair(vertexId, asBegin);
    if (cache.count(key) == 1) {
        return cache[key];
    }

    if (visited[vertexId]) {
        return 0; // loop
    }

    visited[vertexId] = true;

    const auto& vertex = component_->graph_->getVertex(vertexId);
    std::list<Edge*> edges = asBegin ? vertex->getEdgesB() : vertex->getEdgesE();

    int maxLength = vertex->getLength();

    for (const auto& e: edges) {
        bool asBeginNext = asBegin ^ e->getOverlap()->isInnie();
        const auto& nextVertex = e->getDst();

        const int currLen = longestPath(nextVertex->getId(), asBeginNext, cache, visited)
            + vertex->getLength() + e->labelLength() - nextVertex->getLength();

        maxLength = std::max(maxLength, currLen);
    }

    visited[vertexId] = false;

    cache[key] = maxLength;
    return cache[key];
}
