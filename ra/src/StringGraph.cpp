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

const int NOT_FOUND = -1;

inline std::string vertices_sequence_from_walk(const StringGraphWalk* walk);
static int findSingularChain(std::vector<const Edge*>* dst, const Vertex* start, const int start_direction);
static int countForks(const Vertex* start, const int start_direction, const int depth);

//*****************************************************************************
// Edge

Edge::Edge(int id, int readId, const Overlap* overlap, const StringGraph* graph) {

    id_ = id;

    src_ = graph->getVertex(readId);
    dst_ = graph->getVertex(overlap->getA() == readId ? overlap->getB() : overlap->getA());

    overlap_ = overlap;

    graph_ = graph;
    marked_ = false;

    labelLength_ = -1;
}

void Edge::label(std::string& dst) const {

    if (src_->getId() == overlap_->getA()) {
        // from A to B
        int start, len;

        if (overlap_->isInnie()) {

            if (overlap_->isUsingSuffix(dst_->getId())) {
                start = overlap_->getLength(dst_->getId());
                len = overlap_->getBHang();
            } else {
                start = 0;
                len = -1 * overlap_->getAHang();
            }

        } else {

            if (overlap_->isUsingSuffix(dst_->getId())) {
                start = 0;
                len = -1 * overlap_->getAHang();
            } else {
                start = overlap_->getLength(dst_->getId());
                len = overlap_->getBHang();
            }
        }

        dst = (overlap_->isInnie() ? dst_->getReverseComplement() : dst_->getSequence()).substr(start, len);

    } else {
        // from B to A
        int start, len;

        if (overlap_->isUsingSuffix(dst_->getId())) {
            start = 0;
            len = overlap_->getAHang();
        } else {
            start = overlap_->getLength(dst_->getId());
            len = -1 * overlap_->getBHang();
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

    assert(labelLength_ == abs(overlap_->getAHang()) || labelLength_ == abs(overlap_->getBHang()));

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

    if (edge->getOverlap()->isUsingSuffix(this->getId())) {
        edgesE_.emplace_back(edge);
    } else {
        edgesB_.emplace_back(edge);
    }
}


const bool Vertex::isBeginEdge(const Edge* e) const {
  if (e == nullptr) return false;
  if (e->getOverlap()->isUsingSuffix(this->getId())) return false;
  return true;
}

void Vertex::markEdge(int id) {

    for (const auto& edge : edgesE_) {
        if (edge->getId() == id) {
            edge->mark();
            edge->pair_->mark();
            return;
        }
    }

    for (const auto& edge : edgesB_) {
        if (edge->getId() == id) {
            edge->mark();
            edge->pair_->mark();
            return;
        }
    }
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

//*****************************************************************************



//*****************************************************************************
// StringGraph

StringGraph::StringGraph(const std::vector<Read*>& reads, const std::vector<Overlap*>& overlaps) {

    Timer timer;
    timer.start();

    overlaps_ = &overlaps;

    vertices_.reserve(reads.size());

    for (const auto& read : reads) {
        verticesDict_[read->getId()] = vertices_.size();
        vertices_.emplace_back(new Vertex(read->getId(), read, this));
    }

    edges_.reserve(overlaps.size() * 2);

    for (const auto& overlap : overlaps) {
        Edge* edgeA = new Edge(edges_.size(), overlap->getA(), overlap, this);

        edges_.emplace_back(edgeA);
        vertices_[verticesDict_.at(overlap->getA())]->addEdge(edgeA);

        Edge* edgeB = new Edge(edges_.size(), overlap->getB(), overlap, this);

        edges_.emplace_back(edgeB);
        vertices_[verticesDict_.at(overlap->getB())]->addEdge(edgeB);

        edgeA->pair_ = edgeB;
        edgeB->pair_ = edgeA;
    }

    timer.stop();
    timer.print("SG", "construction");
}

StringGraph::~StringGraph() {

    for (const auto& vertex : vertices_) delete vertex;
    for (const auto& edge : edges_) delete edge;
}

void StringGraph::trim() {

    Timer timer;
    timer.start();

    size_t disconnectedNum = 0;
    size_t tipsNum = 0;

    for (const auto& vertex : vertices_) {

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

                const auto& oppositeEdges = edge->getOverlap()->isUsingSuffix(opposite->getId()) ?
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
        deleteMarked();
    }

    fprintf(stderr, "[SG][trimming]: removed %zu tips and %zu disconnected vertices\n",
        tipsNum, disconnectedNum);

    timer.stop();
    timer.print("SG", "trimming");
}

void StringGraph::popBubbles() {

    Timer timer;
    timer.start();

    size_t bubblesPoppedNum = 0;

    for (const auto& vertex : vertices_) {

        if (vertex->isMarked()) {
            continue;
        }

        for (int direction = 0; direction <= 1; ++direction) {

            if (!vertex->isBubbleRootCandidate(direction)) {
                continue;
            }

            std::vector<StringGraphWalk*> walks;
            findBubbleWalks(walks, vertex, direction);

            if (walks.size() == 0) {
                continue;
            }

            if (popBubble(walks, direction)) {
                ++bubblesPoppedNum;
            }

            for (const auto& walk : walks) delete walk;
        }
    }

    if (bubblesPoppedNum > 0) {
        deleteMarked();
    }

    fprintf(stderr, "[SG][bubble popping]: popped %zu bubbles\n", bubblesPoppedNum);

    timer.stop();
    timer.print("SG", "bubble popping");
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

        ++numBubbleRounds;
        popBubbles();

        if (numVerticesTemp == vertices_.size() && numEdgesTemp == edges_.size()) {
            break;
        }
    }

    fprintf(stderr, "[SG][simplification]: number of trimming rounds = %zu\n", numTrimmingRounds);
    fprintf(stderr, "[SG][simplification]: number of bubble popping rounds = %zu\n", numBubbleRounds);

    timer.stop();
    timer.print("SG", "simplification");
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

    for (const auto& vertex : vertices_) {
        maxId = std::max(vertex->getId(), maxId);
    }

    std::vector<bool> used(maxId + 1, false);

    for (const auto& vertex : vertices_) {

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

void StringGraph::findBubbleWalks(std::vector<StringGraphWalk*>& dst, const Vertex* root, int direction) {

    std::map<uint32_t, uint32_t> node_visited;

    // vector that keeps track of all created BFS nodes.
    std::vector<StringGraphNode*> nodes;

    // all active walks
    std::vector<const StringGraphNode*> walks;

    StringGraphNode* rootNode = new StringGraphNode(root, nullptr, nullptr, direction, 0);
    walks.push_back(rootNode);
    nodes.push_back(rootNode);
    node_visited[rootNode->getVertex()->getId()]++;

    bool changed = false;
    int juncture_id = NOT_FOUND;
    do {
      if (nodes.size() > MAX_NODES) {
        break;
      }

      changed = false;

      /*
       * We are getting walks.size() upfront on purpose.
       * That's how we make extending fair for every walk (otherwise would add new walk
       * and extend it in the same round).
       */
      int size = walks.size();
      for (int i = 0; i < size && juncture_id == NOT_FOUND; ++i) {
        auto curr_walk = walks[i];

        int lo = nodes.size();
        int extended_walks = curr_walk->expand(nodes);

        if (extended_walks> 0) {
          // replace current walk with one that's one node longer
          walks[i] = nodes[lo];

          // walk forked so we have to add new walks at the and for the next round
          for (int j = 1; j < extended_walks; ++j) {
            walks.push_back(nodes[lo + j]);
          }

          changed = true;
        }

        // mark nodes as visited
        for (int j = 0; j < extended_walks; ++j) {
          const auto new_walk = nodes[lo + j];
          const auto end_id = new_walk->getVertex()->getId();
          node_visited[end_id]++;

          if (node_visited[end_id] == walks.size()) {
            juncture_id = end_id;
            break;
          }
        }
      }

    } while (changed && juncture_id == NOT_FOUND);

    if (juncture_id != NOT_FOUND) {
      bool has_bubble = true;
      for (uint32_t i = 0; i < walks.size(); ++i) {
        walks[i] = walks[i]->rewindedTo(juncture_id);
        has_bubble = has_bubble && walks[i] != nullptr;
      }

      if (has_bubble) {
        for (uint32_t i = 0; i < walks.size(); ++i) {
          dst.emplace_back(walks[i]->getWalk());
        }
      }
    }

    for (auto n: nodes) {
      delete n;
    }
}

bool StringGraph::popBubble(const std::vector<StringGraphWalk*>& walks, int direction) {

    size_t selectedWalk = 0;
    double selectedCoverage = 0;

    int overlapStart = std::numeric_limits<int>::max();
    int overlapEnd = std::numeric_limits<int>::max();

    size_t i = 0;
    for (const auto& walk : walks) {

        double coverage = 0;
        for (const auto& edge : walk->getEdges()) {
            coverage += edge->getDst()->getCoverage();
        }

        if (coverage > selectedCoverage) {
            selectedWalk = i;
            selectedCoverage = coverage;
        }

        {
          const auto& startEdge = walk->getEdges().front();
          const auto& overlap = startEdge->overlap_;
          overlapStart = std::min(overlapStart, overlap->getLength(startEdge->getSrc()->getId()));
        }

        {
          const auto& endEdge = walk->getEdges().back();
          const auto& overlap = endEdge->overlap_;
          overlapEnd = std::min(overlapEnd, overlap->getLength(endEdge->getDst()->getId()));
        }

        ++i;
    }

    // types: 0 - normal, 1 - reverse complement
    auto getType = [](const Edge* edge, int id) -> int {
        if (edge->getOverlap()->getA() == id) return 0; // due to possible overlap types
        if (!edge->getOverlap()->isInnie()) return 0;
        return 1;
    };

    // maps edge -> number of walks using that edge
    std::map<std::pair<uint32_t, uint32_t>, uint32_t> edge_used;
    auto edge_key = [](const Edge* edge) -> const std::pair<uint32_t, uint32_t> {
      auto src = edge->getSrc()->getId(), dst = edge->getDst()->getId();
      return std::make_pair(std::min(src, dst), std::max(src, dst));
    };

    // fill edge usage map
    for (const auto& walk: walks) {
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
    for (const auto& walk: walks) {
      int external_edges = 0;
      for (const auto& walk_edge: walk->getEdges()) {
        if (walk_edge->isMarked()) continue;

        auto key = edge_key(walk_edge);
        edge_used[key] += external_edges;

        auto v = walk_edge->getDst();
        edge_used[key] += count_external_edges(v, walk_edge);
      }
    }

    std::vector<std::string> sequences;

    for (const auto& walk : walks) {

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
        sequences.push_back(startInverted ? reversedComplement(sequence) : sequence);
    }

    bool popped = false;

    for (size_t i = 0; i < sequences.size(); ++i) {

        if (i == selectedWalk) {
            continue;
        }

        int distance = editDistance(sequences[i], sequences[selectedWalk]);
        if (distance / (double) sequences[selectedWalk].size() >= MAX_DIFFERENCE) {
            auto curr_repr = vertices_sequence_from_walk(walks[i]);
            auto selected_repr = vertices_sequence_from_walk(walks[selectedWalk]);
            debug("KEEPBUBBLE %s because diff with %s is %f > %f\n",
                    curr_repr.c_str(), selected_repr.c_str(),
                    distance / (double) sequences[selectedWalk].size(),
                    MAX_DIFFERENCE
            );

            continue;
        }

        auto curr_repr = vertices_sequence_from_walk(walks[i]);
        debug("RMBUBBLE %s\n", curr_repr.c_str());

        // mark walk for removal
        auto& edges = walks[i]->getEdges();
        for (auto& e: edges) {
          const auto key = edge_key(e);
          edge_used[key]--;

          // remove the edge if that was the last walk using it
          if (edge_used[key] == 0) {
            Edge* edge = const_cast<Edge*>(e);

            // mark edge for removal
            edge->mark();
            edge->pair()->mark();

            marked_.push_back(edge->getSrc()->getId());
            marked_.push_back(edge->getDst()->getId());
          }

          popped = true;
        }
    }

    return popped;
}

void StringGraph::deleteMarked() {

    // remove marked edges which are marked due to deletion of their opposite edges
    for (const auto& id : marked_) {
        vertices_[verticesDict_[id]]->removeMarkedEdges();
    }

    marked_.clear();

    // delete vertices
    std::vector<Vertex*> verticesNew;
    std::map<int, int> verticesDictNew;

    for (const auto& vertex : vertices_) {

        if (vertex->isMarked()) {
            delete vertex;
            continue;
        }

        verticesDictNew[vertex->getId()] = verticesNew.size();
        verticesNew.emplace_back(vertex);
    }

    verticesDict_.swap(verticesDictNew);
    vertices_.swap(verticesNew);

    // delete edges
    std::vector<Edge*> edgesNew;

    for (const auto& edge : edges_) {

        if (edge->isMarked()) {
            delete edge;
            continue;
        }

        edgesNew.emplace_back(edge);
    }

    edges_.swap(edgesNew);
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
        if (edge->getOverlap()->getA() == id) return 0; // due to possible overlap types
        if (!edge->getOverlap()->isInnie()) return 0;
        return 1;
    };

    int startType = getType(edges_.front(), start_->getId());

    bool appendToPrefix = edges_.front()->getOverlap()->isUsingPrefix(start_->getId()) ^ startType;

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
            edge->getOverlap()->isInnie() ? (direction_ ^ 1) : direction_,
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

static int lengthRecursive(const Vertex* vertex, int direction, std::vector<bool>& visited,
    const int branch, const int maxBranch) {

    if (branch > maxBranch) {
        debug("STOPEXPAND %d because hit max branches %d\n", vertex->getReadId(), MAX_BRANCHES);
        return 0;
    }

    if (visited[vertex->getId()]) {
        return 0;
    }

    visited[vertex->getId()] = true;

    const auto& edges = direction == 0 ? vertex->getEdgesB() : vertex->getEdgesE();

    int length = 0;

    if (edges.size() == 1) {

        const auto& edge = edges.front();
        length += edge->labelLength();
        length += lengthRecursive(edge->getDst(), edge->getOverlap()->isInnie() ?
            (direction ^ 1) : direction, visited, branch, maxBranch);

    } else if (edges.size() > 1) {

        int maxLength = 0;
        Edge* selectedEdge = edges.front();

        for (const auto& edge : edges) {

            int len = lengthRecursive(edge->getDst(), edge->getOverlap()->isInnie() ? (direction ^ 1) :
                direction, visited, branch + 1, maxBranch);

            if (len > maxLength) {
                maxLength = len;
                selectedEdge = edge;
            }
        }

        length += selectedEdge->labelLength();
        length += maxLength;
    }

    visited[vertex->getId()] = false;

    return length;
}

static int expandVertex(std::vector<const Edge*>& dst, const Vertex* start, const int start_direction, const int maxId, const int max_branches) {

    int totalLength = start->getLength();
    const Vertex* vertex = start;
    int curr_direction = start_direction;

    std::vector<bool> visitedVertices(maxId + 1, false);

    while (true) {

        if (visitedVertices[vertex->getId()]) {
            break;
        }

        visitedVertices[vertex->getId()] = true;

        const auto& edges = curr_direction == 0 ? vertex->getEdgesB() : vertex->getEdgesE();

        if (edges.size() == 0) {
            break;
        }

        Edge* selectedEdge = edges.front();

        if (edges.size() > 1) {

            double selectedLength = 0;
            int selectedScore = -1;

            for (const auto& edge : edges) {

                const Vertex* next = edge->getDst();

                if (visitedVertices[next->getId()]) {
                    continue;
                }

                debug("EXPAND %d\n", start->getReadId());
                int length = lengthRecursive(next, edge->getOverlap()->isInnie() ? (curr_direction ^ 1) :
                    curr_direction, visitedVertices, 0, max_branches);

                int curr_score = edge->getOverlap()->getScore();
                // TODO: remove magic number
                if ((length > selectedLength && 0.8 * selectedScore < curr_score) ||
                    (curr_score > selectedScore && 0.8 * selectedLength < length)) {
                    selectedEdge = edge;
                    selectedLength = length;
                    selectedScore = curr_score;
                }
            }
        }

        dst.emplace_back(selectedEdge);
        vertex = selectedEdge->getDst();

        totalLength += selectedEdge->labelLength();

        if (selectedEdge->getOverlap()->isInnie()) {
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

        if (selectedEdge->getOverlap()->isInnie()) {
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
      if (e->getOverlap()->isInnie()) {
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
                startCandidates.emplace_back(vertex, direction, lengthRecursive(vertex, direction,
                    visited, 0, 0));
            }
        }
    }

    // forks could be good candidates, too
    for (int direction = 0; direction <= 1; ++direction) {

        for (const auto& vertex : vertices_) {

            if ((direction == 0 && vertex->getEdgesB().size() > 1) ||
                (direction == 1 && vertex->getEdgesE().size() > 1)) {

                std::vector<bool> visited(maxId + 1, false);
                startCandidates.emplace_back(vertex, direction, lengthRecursive(vertex, direction,
                    visited, 0, 1));
            }
        }
    }

    std::sort(startCandidates.begin(), startCandidates.end(),
        [](const Candidate& left, const Candidate& right) {
            return std::get<2>(left) > std::get<2>(right);
        }
    );

    size_t n = std::min(MAX_START_NODES, startCandidates.size());

    // expand each of n candidates to a full chain and pick the best one (by length)
    StringGraphWalk* selectedContig = nullptr;
    int selectedLength = 0;

    for (size_t i = 0; i < n; ++i) {

        const Vertex* start = std::get<0>(startCandidates[i]);
        int direction = std::get<1>(startCandidates[i]);

        std::vector<const Edge*> edges;
        int length = expandVertex(edges, start, direction, maxId, MAX_BRANCHES);

        if (length > selectedLength) {

            selectedLength = length;

            delete selectedContig;

            selectedContig = new StringGraphWalk(start);
            for (const auto& edge : edges) {
                selectedContig->addEdge(edge);
            }
        }
    }

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
