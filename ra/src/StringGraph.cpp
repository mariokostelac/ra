/*!
 * @file StringGraph.cpp
 *
 * @brief StringGraph and other classes source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 30, 2015
 */

#include "EditDistance.hpp"
#include "StringGraph.hpp"

using std::map;
using std::max;
using std::min;
using std::swap;

const int NOT_FOUND = -1;
const int NOT_DEFINED = -1;

inline std::string vertices_sequence_from_walk(const StringGraphWalk* walk);
static int findSingularChain(std::vector<const Edge*>* dst, const Vertex* start, const int start_direction);
static int countForks(const Vertex* start, const int start_direction, const int depth);

//*****************************************************************************
// Edge

Edge::Edge(int id, int readId, const DovetailOverlap* overlap, const StringGraph* graph) {

    id_ = id;

    src_ = graph->getVertex(readId);
    dst_ = graph->getVertex(overlap->a() == readId ? overlap->b() : overlap->a());

    overlap_ = overlap;

    graph_ = graph;
    marked_ = false;

    labelLength_ = -1;
}

void Edge::label(std::string& dst) const {

    if (src_->getId() == overlap_->a()) {
        // from A to B
        int start, len;

        if (overlap_->innie()) {

            if (overlap_->is_using_suffix(dst_->getId())) {
                start = overlap_->length(dst_->getId());
                len = overlap_->b_hang();
            } else {
                start = 0;
                len = -1 * overlap_->a_hang();
            }

        } else {

            if (overlap_->is_using_suffix(dst_->getId())) {
                start = 0;
                len = -1 * overlap_->a_hang();
            } else {
                start = overlap_->length(dst_->getId());
                len = overlap_->b_hang();
            }
        }

        dst = (overlap_->innie() ? dst_->getReverseComplement() : dst_->getSequence()).substr(start, len);

    } else {
        // from B to A
        int start, len;

        if (overlap_->is_using_suffix(dst_->getId())) {
            start = 0;
            len = overlap_->a_hang();
        } else {
            start = overlap_->length(dst_->getId());
            len = -1 * overlap_->b_hang();
        }

        dst = dst_->getSequence().substr(start, len);
    }
}

int Edge::labelLength() {

    if (labelLength_ > -1) {
        return labelLength_;
    }

    std::string l;
    label(l);
    labelLength_ = l.length();

    assert(labelLength_ == abs(overlap_->a_hang()) || labelLength_ == abs(overlap_->b_hang()));

    return labelLength_;
}

void Edge::rkLabel(std::string& dst) const {

    std::string label;
    this->label(label);

    dst = "";

    for (int i = label.size() - 1; i >= 0; --i) {

        char c = label[i];

        switch (c) {
            case 'A':
                c = 'T';
                break;
            case 'T':
                c = 'A';
                break;
            case 'C':
                c = 'G';
                break;
            case 'G':
                c = 'C';
                break;
            default:
                break;
        }

        dst += c;
    }
}

const Vertex* Edge::oppositeVertex(int id) const {

    if (id == src_->getId()) return dst_;
    if (id == dst_->getId()) return src_;

    ASSERT(false, "Vertex", "wrong vertex id");
}

Edge* Edge::pair() const {
    return pair_;
}

//*****************************************************************************



//*****************************************************************************
// Vertex

Vertex::Vertex(int id, const Read* read, const StringGraph* graph) :
    id_(id), read_(read), graph_(graph), marked_(false) {
}

bool Vertex::isTipCandidate() const {
    if (edgesB_.size() == 0 || edgesE_.size() == 0) return true;
    return false;
}

bool Vertex::isBubbleRootCandidate(int direction) const {

    size_t unmarked = 0;

    if (direction == 0) {
        for (const auto& edge : edgesB_) {
            if (!edge->isMarked()) ++unmarked;
        }
    }
    else if (direction == 1) {
        for (const auto& edge : edgesE_) {
            if (!edge->isMarked()) ++unmarked;
        }
    }

    return unmarked > 1;
}

void Vertex::addEdge(Edge* edge) {

    if (edge->getOverlap()->is_using_suffix(this->getId())) {
        edgesE_.emplace_back(edge);
    } else {
        edgesB_.emplace_back(edge);
    }
}


const bool Vertex::isBeginEdge(const Edge* e) const {
  if (e == nullptr) return false;
  if (e->getOverlap()->is_using_suffix(this->getId())) return false;
  return true;
}

void Vertex::markEdges() {

    for (const auto& edge : edgesE_) {
        edge->mark();
        edge->pair_->mark();
    }

    for (const auto& edge : edgesB_) {
        edge->mark();
        edge->pair_->mark();
    }
}

void Vertex::removeMarkedEdges(const bool propagate) {

    std::vector<Vertex *> other_vertices;

    for (auto edge = edgesE_.begin(); edge != edgesE_.end();) {

        if ((*edge)->isMarked()) {
            if (propagate) {
              other_vertices.push_back(const_cast<Vertex*>((*edge)->oppositeVertex(getId())));
            }
            edge = edgesE_.erase(edge);
        } else {
            ++edge;
        }
    }

    for (auto edge = edgesB_.begin(); edge != edgesB_.end();) {

        if ((*edge)->isMarked()) {
            if (propagate) {
              other_vertices.push_back(const_cast<Vertex*>((*edge)->oppositeVertex(getId())));
            }
            edge = edgesB_.erase(edge);
        } else {
            ++edge;
        }
    }

    for (auto v: other_vertices) {
      v->removeMarkedEdges(false);
    }
}

const Edge* Vertex::bestEdge(const bool use_end) const {
  const auto& edges = use_end ? edgesE_ : edgesB_;

  if (edges.size() == 0) {
    return nullptr;
  }

  Edge* best_edge = edges.front();
  int best_length = best_edge->getOverlap()->length(this->getReadId());

  for (auto& edge : edges) {

    int curr_length = edge->getOverlap()->length(this->getReadId());

    if (curr_length > best_length) {
      best_edge = edge;
      best_length = curr_length;
    }
  }

  return best_edge;
}

//*****************************************************************************



//*****************************************************************************
// StringGraph

StringGraph::StringGraph(const std::vector<Read*>& reads, const std::vector<DovetailOverlap*>& overlaps) {

    Timer timer;
    timer.start();

    overlaps_ = &overlaps;

    for (const auto& read : reads) {
        vertices_.emplace(read->getId(), new Vertex(read->getId(), read, this));
    }

    edges_.reserve(overlaps.size() * 2);

    for (const auto& overlap : overlaps) {
        Edge* edgeA = new Edge(edges_.size(), overlap->a(), overlap, this);

        edges_.emplace_back(edgeA);
        vertices_[overlap->a()]->addEdge(edgeA);

        Edge* edgeB = new Edge(edges_.size(), overlap->b(), overlap, this);

        edges_.emplace_back(edgeB);
        vertices_[overlap->b()]->addEdge(edgeB);

        edgeA->pair_ = edgeB;
        edgeB->pair_ = edgeA;
    }

    timer.stop();
    timer.print("SG", "construction");
}

StringGraph::~StringGraph() {

    for (const auto& vertex : vertices_) delete vertex.second;
    for (const auto& edge : edges_)      delete edge;

    vertices_.clear();
    edges_.clear();
}

void StringGraph::trim() {

    Timer timer;
    timer.start();

    size_t disconnectedNum = 0;
    size_t tipsNum = 0;

    for (const auto& kv : vertices_) {
        const auto& vertex = kv.second;

        if (vertex->getLength() > READ_LEN_THRESHOLD) {
            continue;
        }

        // check if disconnected
        if (vertex->getEdgesB().size() == 0 && vertex->getEdgesE().size() == 0) {
            vertex->mark();
            ++disconnectedNum;
            debug("TRIM %d DISCONNECTED\n", vertex->getId());
            continue;
        }

        // check if tip
        if (vertex->isTipCandidate()) {

            const auto& edges = vertex->getEdgesB().size() == 0 ? vertex->getEdgesE() :
                vertex->getEdgesB();

            debug("TIPCANDIDATE %d\n", vertex->getId());

            bool isTip = false;
            for (const auto& edge : edges) {

                // check if opposite vertex has other edges similar as this one

                const auto& opposite = edge->oppositeVertex(vertex->getId());

                const auto& oppositeEdges = edge->getOverlap()->is_using_suffix(opposite->getId()) ?
                    opposite->getEdgesE() : opposite->getEdgesB();

                for (const auto& oedge : oppositeEdges) {
                    if (!oedge->isMarked() && !oedge->oppositeVertex(opposite->getId())->isTipCandidate()) {
                        isTip = true;
                        break;
                    }
                }

                if (isTip) {
                  break;
                }
            }

            if (!isTip) {
              // check if long tip
              std::vector<const Edge*> chain;
              int use_suffix = vertex->getEdgesE().size() ? 1 : 0;
              findSingularChain(&chain, vertex, use_suffix);

              debug("CHAIN %d, FORKS %d\n", chain.size(), countForks(vertex, use_suffix, MAX_DEPTH_WITHOUT_EXTRA_FORK));
              if (chain.size() <= MAX_READS_IN_TIP && countForks(vertex, use_suffix, MAX_DEPTH_WITHOUT_EXTRA_FORK) <= 1) {
                debug("LONG TIP %d\n", vertex->getId());
                isTip = true;
              }

              // maybe filter by seqlen, too
            }

            if (isTip) {
              debug("TRIM %d TIP\n", vertex->getId());
              vertex->mark();
              vertex->markEdges();
              vertex->removeMarkedEdges();

              ++tipsNum;
            }
        }
    }

    if (disconnectedNum > 0 || tipsNum > 0) {
        delete_marked();
    }

    fprintf(stderr, "[SG][trimming]: removed %zu tips and %zu disconnected vertices\n",
        tipsNum, disconnectedNum);

    timer.stop();
    timer.print("SG", "trimming");
}

uint32_t StringGraph::popBubbles() {

    Timer timer;
    timer.start();

    size_t bubblesPoppedNum = 0;

    for (auto& kv : vertices_) {
        auto& vertex = kv.second;

        if (vertex->isMarked()) {
            continue;
        }

        for (int direction = 0; direction <= 1; ++direction) {

            if (!vertex->isBubbleRootCandidate(direction)) {
                continue;
            }

            bubblesPoppedNum += popBubblesStartingAt(vertex, direction);
        }
    }

    if (bubblesPoppedNum > 0) {
        delete_marked();
    }

    fprintf(stderr, "[SG][bubble popping]: popped %zu bubbles\n", bubblesPoppedNum);

    timer.stop();
    timer.print("SG", "bubble popping");

    return bubblesPoppedNum;
}

void StringGraph::simplify() {

    Timer timer;
    timer.start();

    size_t numTrimmingRounds = 0;
    size_t numBubbleRounds = 0;

    size_t numVertices = 0;
    size_t numEdges = 0;

    while (numVertices != vertices_.size() || numEdges != edges_.size()) {

        numVertices = vertices_.size();
        numEdges = edges_.size();

        // trimming
        size_t numVerticesTemp = 0;
        while (numVerticesTemp != vertices_.size()) {

            numVerticesTemp = vertices_.size();

            ++numTrimmingRounds;
            trim();
        }

        // bubble popping
        numVerticesTemp = vertices_.size();
        size_t numEdgesTemp = edges_.size();

        auto MAX_NODES_TMP = MAX_NODES;
        MAX_NODES = min((size_t) 5, MAX_NODES);
        do {
          MAX_NODES = min(MAX_NODES * 2, MAX_NODES_TMP);

          fprintf(stderr, "[SG][bubble popping]: max bracket size %lu\n", MAX_NODES);
          int popped = popBubbles();
          if (popped > 0) {
            break;
          }

        } while (MAX_NODES < MAX_NODES_TMP);

        // bring back original limit
        swap(MAX_NODES, MAX_NODES_TMP);

        ++numBubbleRounds;

        if (numVerticesTemp == vertices_.size() && numEdgesTemp == edges_.size()) {
            break;
        }
    }

    fprintf(stderr, "[SG][simplification]: number of trimming rounds = %zu\n", numTrimmingRounds);
    fprintf(stderr, "[SG][simplification]: number of bubble popping rounds = %zu\n", numBubbleRounds);

    timer.stop();
    timer.print("SG", "simplification");
}

int StringGraph::reduceToBOG() {

  int removed = 0;

  // maps overlap_id -> Vertex if overlap_id is best overlap for read inside Vertex.
  // overlap_id =  min(edge1, edge2); edges represent the same overlap.
  map<uint32_t, Vertex*> best_for;

  for (auto& kv1 : vertices_) {
    auto& v1 = kv1.second;

    for (int use_end = 0; use_end < 2; ++use_end) {

      const auto& best_edge = v1->bestEdge(use_end);
      if (best_edge == nullptr) {
        continue;
      }

      const auto& overlap = best_edge->getOverlap();
      auto overlap_id = min(best_edge->getId(), best_edge->pair()->getId());

      if (best_for.count(overlap_id)) {
        // we found that this overlap is best for two different vertices
        assert("Reads must be different" && v1 != best_for[overlap_id]);

        const auto& v2 = best_edge->getSrc() == v1 ? best_edge->getDst() : best_edge->getSrc();

        debug("RMBADEDGES %d\n", v1->getReadId());
        debug("RMBADEDGES %d\n", v2->getReadId());

        const auto& edges_v1 = use_end ? v1->getEdgesE() : v1->getEdgesB();
        const auto& edges_v2 = use_end ^ overlap->innie() ? v2->getEdgesB() : v2->getEdgesE();

        int kept = 0;

        for (const auto& edge : edges_v1) {

          if (edge->getOverlap() == best_edge->getOverlap()) {
            ++kept;
            continue;
          }

          edge->mark();
          edge->pair()->mark();

          removed += 2;
        }
        assert(kept == 1);

        for (const auto& edge : edges_v2) {

          if (edge->getOverlap() == best_edge->getOverlap()) {
            ++kept;
            continue;
          }

          edge->mark();
          edge->pair()->mark();

          removed += 2;
        }
        assert(kept == 2);

      } else {
        best_for[overlap_id] = v1;
      }

    }
  }

  this->delete_marked();

  trim();

  return removed;
}

void StringGraph::extractOverlaps(std::vector<Overlap*>& dst, bool view) const {

    dst.reserve(edges_.size() / 2);

    for (const auto& edge : edges_) {

        if (edge->getId() % 2 == 1) continue;

        dst.push_back(view ? (*overlaps_)[edge->getId() / 2] :
            (*overlaps_)[edge->getId() / 2]->clone());
    }
}

void StringGraph::extractComponents(std::vector<StringGraphComponent*>& dst) const {

    Timer timer;
    timer.start();

    int maxId = 0;

    for (const auto& kv : vertices_) {
        const auto& vertex = kv.second;
        maxId = std::max(vertex->getId(), maxId);
    }

    std::vector<bool> used(maxId + 1, false);

    for (const auto& kv : vertices_) {
        const auto& vertex = kv.second;

        if (used[vertex->getId()] == true) {
            continue;
        }

        std::vector<int> expanded;
        expanded.emplace_back(vertex->getId());

        std::set<int> componentVertices;
        componentVertices.insert(vertex->getId());

        while (expanded.size() != 0) {

            std::vector<int> newExpanded;

            for (const auto& id : expanded) {

                const auto& eVertex = this->getVertex(id);

                for (const auto& edge : eVertex->getEdgesB()) {
                    auto pair = componentVertices.insert(edge->getDst()->getId());

                    if (pair.second == true) {
                        newExpanded.emplace_back(edge->getDst()->getId());
                    }
                }

                for (const auto& edge : eVertex->getEdgesE()) {
                    auto pair = componentVertices.insert(edge->getDst()->getId());

                    if (pair.second == true) {
                        newExpanded.emplace_back(edge->getDst()->getId());
                    }
                }
            }

            expanded.swap(newExpanded);
        }

        for (const auto& id : componentVertices) {
            used[id] = true;
        }

        dst.emplace_back(new StringGraphComponent(componentVertices, this));
    }

    fprintf(stderr, "[SG][component extraction]: number of components = %zu\n", dst.size());

    timer.stop();
    timer.print("SG", "component extraction");
}

int StringGraph::extract_unitigs(std::vector<StringGraphWalk*>* walks) const {

  int max_id = 0;
  for (auto kv : vertices_) {
    max_id = std::max(kv.second->getId(), max_id);
  }

  // vertex_id -> unitig_id
  std::vector<int> unitig_id(max_id + 1, NOT_DEFINED);
  int curr_unitig_id = 1;

  for (auto kv : vertices_) {

    auto vertex = kv.second;

    if (unitig_id[vertex->getId()] != NOT_DEFINED) continue;

    debug("UNITIGFIND %d ", vertex->getId());

    std::vector<Edge*> edges;

    // mark from vertex to the start of unitig
    mark_unitig(&edges, &unitig_id, curr_unitig_id, vertex, 0);

    // reverse edges
    for (int i = 0, n = edges.size(); i < n; ++i) {
      edges[i] = edges[i]->pair();
    }
    std::reverse(edges.begin(), edges.end());

    // mark from here to the end
    mark_unitig(&edges, &unitig_id, curr_unitig_id, vertex, 1);

    if (edges.size()) {

      walks->emplace_back(new StringGraphWalk(edges.front()->getSrc()));

      for (auto e : edges) {
        walks->back()->addEdge(e);
      }

      debug("UNITIGFOUND %d edges no %lu\n", vertex->getId(), edges.size());

      // create next unitig id
      curr_unitig_id++;
    }
  }

  return curr_unitig_id - 1;
}

uint32_t StringGraph::popBubblesStartingAt(const Vertex* root, int direction) {

    debug("FINDBUBBLE %d\n", root->getId());

    // result -> number of popped walks
    uint32_t popped = 0;

    std::map<uint32_t, uint32_t> node_visited;

    // vector that keeps track of all created BFS nodes.
    std::vector<StringGraphNode*> nodes;

    // heads of all walks
    std::vector<const StringGraphNode*> heads;

    StringGraphNode* rootNode = new StringGraphNode(root, nullptr, nullptr, direction, 0);
    heads.push_back(rootNode);
    nodes.push_back(rootNode);

    bool changed = false;
    do {

      changed = false;

      /*
       * We are getting heads.size() upfront on purpose.
       * That's how we make extending fair for every walk (otherwise would fork head to two heads
       * and extend second one in the same round)
       */
      int size = heads.size();
      int juncture_id = NOT_FOUND;

      for (int i = 0; i < size && juncture_id == NOT_FOUND; ++i) {
        auto curr_head = heads[i];

        int lo = nodes.size();
        int extended_walks = curr_head->expand(nodes);

        if (extended_walks> 0) {
          // replace current head with one that represents longer walk
          heads[i] = nodes[lo];

          // walk forked so we have to add new heads at the and for the next round
          for (int j = 1; j < extended_walks; ++j) {
            heads.push_back(nodes[lo + j]);
          }

          changed = true;
        }

        // mark nodes as visited
        for (int j = 0; j < extended_walks; ++j) {
          const auto new_walk = nodes[lo + j];
          const auto end_id = new_walk->getVertex()->getId();
          node_visited[end_id]++;

          if (node_visited[end_id] == heads.size()) {
            juncture_id = end_id;
            break;
          }
        }
      }

      if (nodes.size() > MAX_NODES) {
        break;
      }

      if (juncture_id != NOT_FOUND) {
        debug("BUBBLEROOT %d JUNCTURE %d\n", root->getId(), juncture_id);

        std::vector<StringGraphWalk*> walks;

        for (uint32_t i = 0; i < heads.size(); ++i) {
          auto rewinded = heads[i]->rewindedTo(juncture_id);
          walks.emplace_back(rewinded ? rewinded->getWalk() : heads[i]->getWalk());
        }

        popped += popBubble(walks, juncture_id, direction);
        break;
      }

    } while (changed && popped == 0);

    for (auto n: nodes) {
      delete n;
    }

    return popped;
}


bool StringGraph::popBubble(const std::vector<StringGraphWalk*>& all_walks, const uint32_t juncture_id, const int direction) {

    // types: 0 - normal, 1 - reverse complement
    auto getType = [](const Edge* edge, int id) -> int {
        if (edge->getOverlap()->a() == id) return 0; // due to possible overlap types
        if (!edge->getOverlap()->innie()) return 0;
        return 1;
    };

    // maps edge -> number of walks using that edge
    std::map<uint32_t, uint32_t> edge_used;
    auto edge_key = [](const Edge* edge) -> uint32_t {
      return std::min(edge->getId(), edge->pair()->getId());
    };

    // fill edge usage map
    for (const auto& walk: all_walks) {
      for (const auto& e: walk->getEdges()) {
        edge_used[edge_key(e)]++;
      }
    }

    auto count_external_edges = [&edge_used, &edge_key](const Vertex* v, const Edge* incoming_edge) -> int {
      int external_edges = 0;

      // get incoming all incoming edges
      auto v_edges = v->isBeginEdge(incoming_edge) ? v->getEdgesB() : v->getEdgesE();
      for (const auto& v_edge: v_edges) {
        // edge is not part of any walk -> external edge
        if (edge_used.count(edge_key(v_edge)) == 0) {
          external_edges++;
        }
      }

      return external_edges;
    };

    // add extra usage if walk has external inbound edges
    for (const auto& walk: all_walks) {

      int external_edges_before = 0;

      for (const auto& walk_edge: walk->getEdges()) {
        if (walk_edge->isMarked()) continue;

        auto key = edge_key(walk_edge);

        edge_used[key] += external_edges_before;

        auto v = walk_edge->getDst();
        edge_used[key] += count_external_edges(v, walk_edge);

        external_edges_before += count_external_edges(v, walk_edge);
      }
    }

    std::vector<StringGraphWalk*> bubble_walks;
    std::vector<std::string> sequences;

    for (const auto& walk : all_walks) {
      if (walk->getEdges().back()->getDst()->getId() == (int) juncture_id) {
        bubble_walks.push_back(walk);
      }
    }

    assert("we need at least two bubble walks" && bubble_walks.size() >= 2);

    size_t selectedWalk = 0;
    double selectedCoverage = 0;

    int overlapStart = std::numeric_limits<int>::max();
    int overlapEnd = std::numeric_limits<int>::max();

    size_t i = 0;
    for (const auto& walk : bubble_walks) {

        double coverage = 0;
        for (const auto& edge : walk->getEdges()) {
            coverage += edge->getDst()->getCoverage();
        }

        if (coverage > selectedCoverage) {
            selectedWalk = i;
            selectedCoverage = coverage;
        }

        assert("bubblewalk is not empty" && walk->getEdges().size() > 0);

        {
          const auto& startEdge = walk->getEdges().front();
          const auto& overlap = startEdge->overlap_;
          overlapStart = std::min(overlapStart, (int) overlap->length(startEdge->getSrc()->getId()));
        }

        {
          const auto& endEdge = walk->getEdges().back();
          const auto& overlap = endEdge->overlap_;
          overlapEnd = std::min(overlapEnd, (int) overlap->length(endEdge->getDst()->getId()));
        }

        ++i;
    }

    for (const auto& walk : bubble_walks) {

        // TODO: consider returning this trimming logic back
        //const Vertex* root = walk->getStart();
        //const Vertex* juncture = walk->getEdges().back()->getDst();

        int startInverted = getType(walk->getEdges().front(), walk->getStart()->getId());

        std::string sequence;
        walk->extractSequence(sequence);

        //int start, end;

        //if (direction == 0) {
            //// start     --------
            //// end   -------
            //// full  ------------
            //// out      ------
            //start = juncture->getLength() - overlapEnd;
            //end = sequence.size() - (root->getLength() - overlapStart);

        //} else {
            //// start --------
            //// end        -------
            //// full  ------------
            //// out      ------
            //start = root->getLength() - overlapStart;
            //end = sequence.size() - (juncture->getLength() - overlapEnd);
        //}

        //sequences.emplace_back(end > start ? sequence.substr(start, end - start) : std::string());
        sequences.push_back(startInverted ? reverse_complement(sequence) : sequence);
    }

    bool popped = false;

    for (size_t i = 0; i < sequences.size(); ++i) {

        if (i == selectedWalk) {
            continue;
        }

        auto smaller = min(sequences[i].size(), sequences[selectedWalk].size());
        auto bigger = max(sequences[i].size(), sequences[selectedWalk].size());
        if ((bigger - smaller) / (double) bigger >= MAX_DIFFERENCE) {
            auto curr_repr = vertices_sequence_from_walk(bubble_walks[i]);
            auto selected_repr = vertices_sequence_from_walk(bubble_walks[selectedWalk]);
            debug("KEEPBUBBLE %s because len diff with %s is %f > %f\n",
                    curr_repr.c_str(), selected_repr.c_str(),
                    sequences[i].size() / (double) sequences[selectedWalk].size(),
                    MAX_DIFFERENCE
            );

            continue;
        }

        int distance = editDistance(sequences[i], sequences[selectedWalk]);
        if (distance / (double) sequences[selectedWalk].size() >= MAX_DIFFERENCE) {
            auto curr_repr = vertices_sequence_from_walk(bubble_walks[i]);
            auto selected_repr = vertices_sequence_from_walk(bubble_walks[selectedWalk]);
            debug("KEEPBUBBLE %s because diff with %s is %f > %f\n",
                    curr_repr.c_str(), selected_repr.c_str(),
                    distance / (double) sequences[selectedWalk].size(),
                    MAX_DIFFERENCE
            );

            continue;
        }

        auto curr_repr = vertices_sequence_from_walk(bubble_walks[i]);

        // mark walk for removal
        auto& edges = bubble_walks[i]->getEdges();
        for (auto& e: edges) {
          const auto key = edge_key(e);
          edge_used[key]--;

          assert(edge_used[key] >= 0);

          // remove the edge if that was the last walk using it
          if (edge_used[key] == 0) {
            Edge* edge = const_cast<Edge*>(e);

            // mark edge for removal
            edge->mark();
            edge->pair()->mark();

            popped = true;
          }
        }

        if (popped) {
          debug("RMBUBBLE %s\n", curr_repr.c_str());
        }
    }

    return popped;
}

int StringGraph::mark_unitig(std::vector<Edge*>* dst_edges, std::vector<int>* unitig_id,
    const int id, const Vertex* start, const int start_direction) const {

  int marked = 0;
  int use_suffix = start_direction;

  auto curr_vertex = start;

  while (true) {

    assert(unitig_id->at(curr_vertex->getId()) == NOT_DEFINED || unitig_id->at(curr_vertex->getId()) == id);
    (*unitig_id)[curr_vertex->getId()] = id;

    marked++;

    auto edge = curr_vertex->bestEdge(use_suffix);

    if (edge == nullptr) {
      break;
    }

    if (edge->getOverlap()->innie()) {
      use_suffix = 1 - use_suffix;
    }

    auto next = edge->getDst();

    // if curr and next do not share best overlap
    if (next->bestEdge(1 - use_suffix)->getOverlap() != edge->getOverlap()) {
      break;
    }

    dst_edges->push_back(const_cast<Edge*>(edge));

    // if read is already part of some other unitig
    if (unitig_id->at(next->getId()) != NOT_DEFINED) {
      break;
    }

    curr_vertex = next;
  }

  return marked;
}

void StringGraph::delete_marked() {
  delete_marked_vertices();
  delete_marked_edges();
}

void StringGraph::delete_marked_edges() {
  std::set<int> dirty_vertices;
  std::vector<Edge*> removed_edges;

  int confirmed = 0;
  for (int i = 0, n = edges_.size(); i < n; ++i) {
    Edge* edge = edges_[i];
    if (edge->isMarked()) {
      dirty_vertices.insert(edge->getSrc()->getId());
      dirty_vertices.insert(edge->getDst()->getId());
      removed_edges.push_back(edge);
      continue;
    }

    edges_[confirmed] = edges_[i];
    confirmed++;
  }
  edges_.resize(confirmed);

  for (auto idx : dirty_vertices) {
      auto vertex = getVertex(idx);
      if (vertex != nullptr) {
        getVertex(idx)->removeMarkedEdges();
      }
  }

  for (Edge* edge : removed_edges) {
    delete edge;
  }
}

void StringGraph::delete_marked_vertices() {
    std::vector<int> for_removal;

    for (auto it : vertices_) {
        Vertex* v = it.second;
        if (!v->isMarked()) {
          continue;
        }

        for (auto edge : v->getEdgesB()) {
          edge->mark();
          edge->pair()->mark();
        }
        for (auto edge : v->getEdgesE()) {
          edge->mark();
          edge->pair()->mark();
        }
        for_removal.push_back(v->getId());
    }

    for (int idx : for_removal) {
      auto it = vertices_.find(idx);
      auto v = it->second;
      vertices_.erase(it);
      delete v;
    }
}

// StringGraphWalk

StringGraphWalk::StringGraphWalk(const Vertex* start) :
    start_(start) {
}

void StringGraphWalk::addEdge(const Edge* edge) {

    edges_.emplace_back(edge);
    visitedVertices_.insert(edge->getDst()->getId());
    visitedEdges_.insert(edge->getId());
}

void StringGraphWalk::extractSequence(std::string& dst) const {

  if (edges_.empty()) {
    dst = std::string(start_->getSequence());
    return;
  }

  // types: 0 - normal, 1 - reverse complement
  auto getType = [](const Edge* edge, int id) -> int {
    if (edge->getOverlap()->a() == id) return 0; // due to possible overlap types
    if (!edge->getOverlap()->innie()) return 0;
    return 1;
  };

  int startType = getType(edges_.front(), start_->getId());

  bool appendToPrefix = edges_.front()->getOverlap()->is_using_prefix(start_->getId()) ^ startType;

  std::string startSequence = std::string(startType ? start_->getReverseComplement() : start_->getSequence());

  // add start vertex
  dst = appendToPrefix ? std::string(startSequence.rbegin(), startSequence.rend()) : startSequence;

  int prevType = startType;

  // add edge labels
  for (const auto& edge : edges_) {

    int type = getType(edge, edge->getSrc()->getId());

    bool invert = type == prevType ? false : true;

    std::string label;
    if (invert) {
      edge->rkLabel(label);
    } else {
      edge->label(label);
    }

    dst += appendToPrefix ? std::string(label.rbegin(), label.rend()) : label;

    prevType = getType(edge, edge->getDst()->getId()) ^ invert;
  }

  if (appendToPrefix) dst = std::string(dst.rbegin(), dst.rend());
}

void StringGraphWalk::extractVertices(std::vector<const Vertex*>& dst) const {
    // add start vertex
    dst.push_back(start_);

    // add edge labels
    for (const auto& edge : edges_) {
        dst.push_back(edge->getDst());
    }
}

bool StringGraphWalk::containsVertex(int id) const {
    return visitedVertices_.count(id) > 0;
}

bool StringGraphWalk::containsEdge(int id) const {
    return visitedEdges_.count(id) > 0;
}

// StringGraphNode

StringGraphNode::StringGraphNode(const Vertex* vertex, const Edge* edgeFromParent, const StringGraphNode* parent,
        int direction, int distance) :
    vertex_(vertex), edgeFromParent_(edgeFromParent), parent_(parent), direction_(direction) {

    if (parent_ == nullptr) {
        distance_ = 0;
    } else {
        distance_ = parent_->distance_ + distance;
    }
}

size_t StringGraphNode::expand(std::vector<StringGraphNode*>& queue) const {

    const auto& edges = direction_ == 0 ? vertex_->getEdgesB() : vertex_->getEdgesE();

    int added = 0;
    for (auto& edge : edges) {

        if (edge->isMarked()) {
            continue;
        }

        queue.emplace_back(new StringGraphNode(edge->getDst(), edge, this,
            edge->getOverlap()->innie() ? (direction_ ^ 1) : direction_,
            edge->labelLength()));
        added++;
    }

    return added;
}

bool StringGraphNode::isInWalk(const StringGraphNode* node) const {

    if (node == nullptr) return false;
    if (node->getVertex()->getId() == this->getVertex()->getId() && node->getParent() != nullptr) return true;
    return this->isInWalk(node->getParent());
}

const StringGraphNode* StringGraphNode::rewindedTo(const int vertexId) const {

    if (vertexId == this->getVertex()->getId()) return this;
    if (this->getParent() == nullptr) return nullptr;
    return this->getParent()->rewindedTo(vertexId);
}

const StringGraphNode* StringGraphNode::findInWalk(const StringGraphNode* node) const {

    if (node == nullptr) return nullptr;
    if (node->getVertex()->getId() == this->getVertex()->getId()) return node;
    return this->findInWalk(node->getParent());
}

StringGraphWalk* StringGraphNode::getWalk() const {
    if (getParent() == nullptr) {
        return new StringGraphWalk(getVertex());
    }

    auto walk = getParent()->getWalk();
    walk->addEdge(getEdgeFromParent());

    return walk;
}

// StringGraphComponent

double overlap_score(const DovetailOverlap* overlap) {
  double quality = 1 - overlap->errate();
  return (overlap->covered_percentage(overlap->a()) + overlap->covered_percentage(overlap->b())) * quality;
};

static int longest_sequence_length(const Vertex* from, const int direction, std::vector<bool>& visited,
    const int forks_left) {

    if (forks_left < 0) {
        debug("STOPEXPAND %d because hit max branches %d\n", from->getReadId(), MAX_BRANCHES);
        return 0;
    }

    if (visited[from->getId()]) {
        debug("STOPEXPAND %d because visited\n", from->getReadId());
        return 0;
    }

    visited[from->getId()] = true;

    const auto& edges = direction == 0 ? from->getEdgesB() : from->getEdgesE();

    int res_length = 0;

    if (edges.size() == 1) {

        const auto& edge = edges.front();
        res_length += edge->labelLength();
        res_length += longest_sequence_length(edge->getDst(), edge->getOverlap()->innie() ?
            (direction ^ 1) : direction, visited, forks_left);

    } else if (edges.size() > 1) {

        Edge* best_edge = nullptr;

        int best_len = 0;
        double best_qual = 0;
        double qual_lo = 0;

        for (const auto& edge : edges) {
          best_qual = max(best_qual, overlap_score(edge->getOverlap()));
        }

        qual_lo = best_qual * (1 - QUALITY_THRESHOLD);

        for (const auto& edge : edges) {

            auto curr_qual = overlap_score(edge->getOverlap());

            if (curr_qual >= qual_lo) {
              int curr_len = longest_sequence_length(edge->getDst(), edge->getOverlap()->innie() ? (direction ^ 1) :
                  direction, visited, forks_left - 1);

              if (curr_len > best_len) {
                best_edge = edge;
                best_len = curr_len;
              }
            }
        }

        if (best_edge != nullptr) {
            res_length += best_edge->labelLength();
            res_length += best_len;
        }
    }

    visited[from->getId()] = false;

    return res_length;
}

static int expandVertex(std::vector<const Edge*>& dst, const Vertex* start, const int start_direction, const int maxId, const int max_branches) {

    debug("EXPAND %d\n", start->getReadId());

    int totalLength = start->getLength();
    const Vertex* vertex = start;
    int curr_direction = start_direction;

    std::vector<bool> visitedVertices(maxId + 1, false);

    while (true) {

        visitedVertices[vertex->getId()] = true;

        const auto& edges = curr_direction == 0 ? vertex->getEdgesB() : vertex->getEdgesE();

        Edge* best_edge = nullptr;
        if (edges.size() == 1) {

            if (!visitedVertices[edges.front()->getDst()->getId()]) {
                best_edge = edges.front();
            }

        } else if (edges.size() > 1) {

            int best_length = 0;
            double best_qual = 0;
            double qual_lo = 0;

            for (const auto& edge : edges) {
              best_qual = max(best_qual, overlap_score(edge->getOverlap()));
            }

            qual_lo = best_qual * (1 - QUALITY_THRESHOLD);

            for (const auto& edge : edges) {

                const Vertex* next = edge->getDst();

                if (visitedVertices[next->getId()]) {
                    continue;
                }

                double curr_qual = overlap_score(edge->getOverlap());
                if (curr_qual >= qual_lo) {
                  int curr_length = longest_sequence_length(next, edge->getOverlap()->innie() ? (curr_direction ^ 1) :
                      curr_direction, visitedVertices, max_branches) + vertex->getLength() + edge->labelLength();

                  if (curr_length > best_length) {
                    best_edge = edge;
                    best_length = curr_length;
                  }
                }
            }
        }

        if (best_edge == nullptr) {
            break;
        }

        dst.emplace_back(best_edge);
        vertex = best_edge->getDst();

        totalLength += best_edge->labelLength();

        if (best_edge->getOverlap()->innie()) {
            curr_direction ^= 1;
        }
    }

    return totalLength;
}

static int findSingularChain(std::vector<const Edge*>* dst, const Vertex* start, const int start_direction) {

    int totalLength = start->getLength();
    const Vertex* curr_vertex = start;
    int curr_direction = start_direction;

    while (true) {

        const auto& edges = curr_direction == 0 ? curr_vertex->getEdgesB() : curr_vertex->getEdgesE();

        if (edges.size() == 0) {
            // end of chain
            break;
        }

        if (curr_vertex->getEdgesB().size() + curr_vertex->getEdgesE().size() > 2) {
            // not singular chain anymore
            break;
        }

        Edge* selectedEdge = edges.front();

        if (dst != nullptr) dst->emplace_back(selectedEdge);
        curr_vertex = selectedEdge->getDst();

        totalLength += selectedEdge->labelLength();

        if (selectedEdge->getOverlap()->innie()) {
            curr_direction ^= 1;
        }
    }

    return totalLength;
}

static int countForks(const Vertex* start, const int start_direction, const int depth) {

    if (depth <= 0) {
      return 0;
    }

    int forks = 0;
    const Vertex* curr_vertex = start;
    int curr_direction = start_direction;

    const auto& edges = curr_direction == 0 ? curr_vertex->getEdgesB() : curr_vertex->getEdgesE();

    if (curr_vertex->getEdgesB().size() + curr_vertex->getEdgesE().size() > 2) {
      forks++;
    }

    for (auto e: edges) {
      curr_vertex = e->getDst();
      if (e->getOverlap()->innie()) {
        curr_direction ^= 1;
      }

      forks += countForks(curr_vertex, curr_direction, depth - 1);
    }

    return forks;
}

StringGraphComponent::StringGraphComponent(const std::set<int> vertexIds, const StringGraph* graph) :
    vertices_(), graph_(graph) {

    for (const auto& id : vertexIds) {
        vertices_.emplace_back(graph->getVertex(id));
    }

    walk_ = nullptr;
}

StringGraphComponent::~StringGraphComponent() {
    delete walk_;
}

void StringGraphComponent::extractSequence(std::string& dst) {

    if (walk_ == nullptr) extractLongestWalk();
    if (walk_ != nullptr) walk_->extractSequence(dst);
}

StringGraphWalk* StringGraphComponent::longestWalk() {
  if (walk_ == nullptr) extractLongestWalk();
  return walk_;
}

void StringGraphComponent::extractLongestWalk() {

    typedef std::tuple<const Vertex*, int, double> Candidate;

    // pick n start vertices based on total coverage of their chains to first branch
    std::vector<Candidate> startCandidates;

    int maxId = 0;
    for (const auto& vertex : vertices_) {
        maxId = std::max(maxId, vertex->getId());
    }

    // tips and singular chains could be good candidates
    for (int direction = 0; direction <= 1; ++direction) {

        for (const auto& vertex : vertices_) {

            if ((direction == 0 && vertex->getEdgesB().size() == 1 && vertex->getEdgesE().size() == 0) ||
                (direction == 1 && vertex->getEdgesE().size() == 1 && vertex->getEdgesB().size() == 0)) {

                std::vector<bool> visited(maxId + 1, false);
                startCandidates.emplace_back(vertex, direction, longest_sequence_length(vertex, direction,
                    visited, 0));
            }
        }
    }

    // forks could be good candidates, too
    for (int direction = 0; direction <= 1; ++direction) {

        for (const auto& vertex : vertices_) {

            if ((direction == 0 && vertex->getEdgesB().size() > 1) ||
                (direction == 1 && vertex->getEdgesE().size() > 1)) {

                std::vector<bool> visited(maxId + 1, false);
                startCandidates.emplace_back(vertex, direction, longest_sequence_length(vertex, direction,
                    visited, 1));
            }
        }
    }

    // circular component
    if (startCandidates.size() == 0) {
      std::vector<bool> visited(maxId + 1, false);

      int direction = 0;
      const auto vertex = vertices_.front();

      startCandidates.emplace_back(vertex, direction, longest_sequence_length(vertex, direction,
            visited, 1));
    }

    std::sort(startCandidates.begin(), startCandidates.end(),
        [](const Candidate& left, const Candidate& right) {
            return std::get<2>(left) > std::get<2>(right);
        }
    );

    int n = std::min(MAX_START_NODES, startCandidates.size());

    // expand each of n candidates to a full chain and pick the best one (by length)
    StringGraphWalk* selectedContig = nullptr;
    int selectedLength = 0;

    Timer timer;
    timer.start();

    #pragma omp parallel for default(none), shared(maxId, n, MAX_BRANCHES, startCandidates, selectedLength, selectedContig), schedule(dynamic, 1)
    for (int i = 0; i < n; ++i) {

        const Vertex* start = std::get<0>(startCandidates[i]);
        int direction = std::get<1>(startCandidates[i]);

        debug("CREATECONTIG from vertex %d\n", start->getId());

        std::vector<const Edge*> edges;
        int length = expandVertex(edges, start, direction, maxId, MAX_BRANCHES);

        #pragma omp critical
        if (length > selectedLength) {

            selectedLength = length;

            if (selectedContig != nullptr) {
              delete selectedContig;
            }

            selectedContig = new StringGraphWalk(start);
            for (const auto& edge : edges) {
                selectedContig->addEdge(edge);
            }
        }
    }

    timer.stop();
    timer.print("SG", "extract longest walk");

    walk_ = selectedContig;
}

// StringGraphComponent

// debug util functions
inline std::string vertices_sequence_from_walk(const StringGraphWalk* walk) {
    std::string dst = "";

    std::vector<const Vertex*> vertices;
    walk->extractVertices(vertices);
    for (const auto v: vertices) {
        dst += std::to_string(v->getReadId());
        dst += " ";
    }

    return dst;
}
