#include "StringGraphUtils.hpp"

using std::vector;

uint32_t mark_external_unitig_edges(StringGraphWalk* walk, StringGraph* graph, std::set<int>& unitig_vertices) {
  uint32_t marked = 0;

  debug("MARK_UNITIG %d\n", walk->getStart()->getId());

  auto edges = walk->getEdges();

  if (edges.size() == 0) {
    return 0;
  }

  auto mark_edges = [&unitig_vertices](const Edge* prev_edge, const Edge *curr_edge, std::list<Edge*>& edges) {
    uint32_t  marked = 0;

    for (auto e : edges) {

      if (prev_edge != nullptr && e->getOverlap() == prev_edge->getOverlap()) {
        continue;
      }

      if (curr_edge != nullptr && e->getOverlap() == curr_edge->getOverlap()) {
        continue;
      }

      if (unitig_vertices.count(e->getDst()->getId())){
        continue;
      }

      debug("MARK_EDGE %d %d\n", e->getSrc()->getId(), e->getDst()->getId());

      e->mark();
      e->pair()->mark();
      marked++;
    }

    return marked;
  };

  const Vertex* curr_vertex = walk->getStart();
  Edge* prev_edge = nullptr;
  for (auto curr_edge : edges) {

    if (curr_edge != edges.front()) {
      auto edges_begin = curr_vertex->getEdgesB();
      marked += mark_edges(prev_edge, curr_edge, edges_begin);

      auto edges_end = curr_vertex->getEdgesE();
      marked += mark_edges(prev_edge, curr_edge, edges_end);
    }

    curr_vertex = curr_edge->oppositeVertex(curr_vertex->getId());
    prev_edge = const_cast<Edge*>(curr_edge);
  }

  //auto last_edges = curr_vertex->isBeginEdge(prev_edge) ? curr_vertex->getEdgesB() : curr_vertex->getEdgesE();
  //marked += mark_edges(nullptr, prev_edge, last_edges);

  return marked;
}

uint32_t remove_external_unitig_edges(StringGraph* graph) {

  uint32_t marked = 0;

  vector<StringGraphWalk*> walks;
  graph->extract_unitigs(&walks);

  std::vector<const Vertex*> unitig_vertices_v;
  for (auto w : walks) {
    w->extractVertices(unitig_vertices_v);
  }

  std::set<int> unitig_vertices;
  for (auto v : unitig_vertices_v) {
    unitig_vertices.insert(v->getId());
  }

  for (auto w : walks) {
    marked += mark_external_unitig_edges(w, graph, unitig_vertices);
  }

  graph->delete_marked();

  return marked;
}

