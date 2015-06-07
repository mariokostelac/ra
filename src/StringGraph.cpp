/*
* StringGraph.cpp
*
* Created on: May 30, 2015
*     Author: rvaser
*/

#include "StringGraph.hpp"

static const size_t MAX_NODES = 500;
static const int MAX_DISTANCE = 1000;
static const double MAX_DIFFERENCE = 0.1;

//*****************************************************************************
// Edge

/*bool compareEdges(const Edge* left, const Edge* right) {

    if (left->getA()->getId() != right->getA()->getId()) {
        return left->getA()->getId() < right->getA()->getId();
    }

    if (left->getB()->getId() != right->getB()->getId()) {
        return left->getB()->getId() < right->getB()->getId();
    }

    return false;
}*/

Edge::Edge(int id, int readId, const Overlap* overlap, const StringGraph* graph) {

    id_ = id;

    a_ = graph->getVertex(readId);
    b_ = graph->getVertex(overlap->getA() == readId ? overlap->getB() : overlap->getA());

    overlap_ = overlap;
    direction_ = overlap->getA() == readId ? Direction::A_TO_B : Direction::B_TO_A;

    graph_ = graph;
    marked_ = false;
}

void Edge::label(std::string& dst) const {

    if (a_->getId() == overlap_->getA()) {
        // from A to B
        int start, len;

        if (overlap_->isInnie()) {

            if (overlap_->isUsingSuffix(b_->getId())) {
                start = overlap_->getLength();
                len = overlap_->getBHang();
            } else {
                start = 0;
                len = -1 * overlap_->getAHang();
            }

        } else {

            if (overlap_->isUsingSuffix(b_->getId())) {
                start = 0;
                len = -1 * overlap_->getAHang();
            } else {
                start = overlap_->getLength();
                len = overlap_->getBHang();
            }
        }

        dst = (overlap_->isInnie() ? b_->getReverseComplement() : b_->getSequence()).substr(start, len);

    } else {
        // from B to A
        int start, len;

        if (overlap_->isUsingSuffix(b_->getId())) {
            start = 0;
            len = overlap_->getAHang();
        } else {
            start = overlap_->getLength();
            len = -1 * overlap_->getBHang();
        }

        dst = b_->getSequence().substr(start, len);
    }
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

    if (id == a_->getId()) return b_;
    if (id == b_->getId()) return a_;

    ASSERT(false, "Vertex", "wrong vertex id");
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

void Vertex::markEdge(int id) {

    for (const auto& edge : edgesE_) {
        if (edge->getId() == id) {
            edge->mark();
            edge->opposite_->mark();
            return;
        }
    }

    for (const auto& edge : edgesB_) {
        if (edge->getId() == id) {
            edge->mark();
            edge->opposite_->mark();
            return;
        }
    }
}

void Vertex::markEdges() {

    for (const auto& edge : edgesE_) {
        edge->mark();
        edge->opposite_->mark();
    }

    for (const auto& edge : edgesB_) {
        edge->mark();
        edge->opposite_->mark();
    }
}

void Vertex::removeMarkedEdges() {

    for (auto edge = edgesE_.begin(); edge != edgesE_.end();) {

        if ((*edge)->isMarked()) {
            edge = edgesE_.erase(edge);
        } else {
            ++edge;
        }
    }

    for (auto edge = edgesB_.begin(); edge != edgesB_.end();) {

        if ((*edge)->isMarked()) {
            edge = edgesB_.erase(edge);
        } else {
            ++edge;
        }
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
        size_t id = vertices_.size();
        verticesDict_[id] = id;
        vertices_.emplace_back(new Vertex(id, read, this));
    }

    edges_.reserve(overlaps.size() * 2);

    for (const auto& overlap : overlaps) {
        Edge* edgeA = new Edge(edges_.size(), overlap->getA(), overlap, this);

        edges_.emplace_back(edgeA);
        vertices_[verticesDict_.at(overlap->getA())]->addEdge(edgeA);

        Edge* edgeB = new Edge(edges_.size(), overlap->getB(), overlap, this);

        edges_.emplace_back(edgeB);
        vertices_[verticesDict_.at(overlap->getB())]->addEdge(edgeB);

        edgeA->opposite_ = edgeB;
        edgeB->opposite_ = edgeA;
    }

    // std::sort(edges_.begin(), edges_.end(), compareEdges);

    timer.stop();
    timer.print("SG", "construction");
}

StringGraph::~StringGraph() {

    for (const auto& vertex : vertices_) delete vertex;
    for (const auto& edge : edges_) delete edge;
}

void StringGraph::trim(int threshold) {

    Timer timer;
    timer.start();

    size_t disconnectedNum = 0;
    size_t tipsNum = 0;

    for (const auto& vertex : vertices_) {

        if (vertex->getLength() > threshold) {
            continue;
        }

        // check if disconnected
        if (vertex->getEdgesB().size() == 0 && vertex->getEdgesE().size() == 0) {
            vertex->mark();
            ++disconnectedNum;
            continue;
        }

        // check if tip
        if (vertex->isTipCandidate()) {

            const auto& edges = vertex->getEdgesB().size() == 0 ? vertex->getEdgesE() :
                vertex->getEdgesB();

            for (const auto& edge : edges) {

                // check if opposite vertex has other edges similar as this one

                const auto& opposite = edge->oppositeVertex(vertex->getId());

                const auto& oppositeEdges = edge->getOverlap()->isUsingSuffix(opposite->getId()) ?
                    opposite->getEdgesE() : opposite->getEdgesB();

                bool isTip = false;

                for (const auto& oedge : oppositeEdges) {
                    if (!oedge->isMarked() && !oedge->oppositeVertex(opposite->getId())->isTipCandidate()) {
                        isTip = true;
                        break;
                    }
                }

                if (isTip) {
                    vertex->mark();
                    vertex->markEdges();
                    ++tipsNum;

                    break;
                }
            }

            if (vertex->isMarked()) {
                for (const auto& edge : edges) {
                    marked_.emplace_back(edge->oppositeVertex(vertex->getId())->getId());
                }
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

void StringGraph::extractOverlaps(std::vector<Overlap*>& dst, bool view) const {

    dst.reserve(edges_.size() / 2);

    for (const auto& edge : edges_) {

        if (edge->getId() % 2 == 1) continue;

        dst.push_back(view ? (*overlaps_)[edge->getId() / 2] :
            (*overlaps_)[edge->getId() / 2]->clone());
    }
}

void StringGraph::findBubbleWalks(std::vector<StringGraphWalk*>& dst, const Vertex* root, int dir) {

    openedQueue_.clear();
    closedQueue_.clear();
    nodes_.clear();

    // BFS search the string graph
    StringGraphNode* rootNode = new StringGraphNode(root, nullptr, nullptr, dir, 0);

    openedQueue_.emplace_back(rootNode);
    nodes_.emplace_back(rootNode);

    std::vector<StringGraphWalk*> walks;

    while (!openedQueue_.empty()) {

        if (nodes_.size() > MAX_NODES) {
            break;
        }

        std::deque<StringGraphNode*> expandQueue;

        while (!openedQueue_.empty()) {

            StringGraphNode* currentNode = openedQueue_.front();
            openedQueue_.pop_front();

            if (currentNode->getDistance() < MAX_DISTANCE) {

                size_t num = currentNode->expand(expandQueue);
                if (num == 0) closedQueue_.emplace_back(currentNode);

            } else {
                closedQueue_.emplace_back(currentNode);
            }
        }

        openedQueue_.swap(expandQueue);
        nodes_.insert(nodes_.end(), openedQueue_.begin(), openedQueue_.end());

        const StringGraphNode* junctureNode = bubbleJuncture(rootNode);

        if (junctureNode != nullptr) {
            extractBubbleWalks(walks, root, junctureNode);
            break;
        }
    }

    for (const auto& it : nodes_) delete it;

    if (walks.size() < 2) {
        for (const auto& walk : walks) delete walk;
        return;
    }

    // check the orientation of juncture vertex overlap of all walks
    // (might be different due to innie overlaps)

    std::vector<StringGraphWalk*> walks0;
    std::vector<StringGraphWalk*> walks1;

    for (size_t i = 0; i < walks.size(); ++i) {

        const Edge* lastEdge = walks[i]->getEdges().back();

        if (lastEdge->getOverlap()->isUsingSuffix(lastEdge->getB()->getId())) {
            walks1.emplace_back(walks[i]);
        } else {
            walks0.emplace_back(walks[i]);
        }
    }

    if (walks0.size() > 1) {
        for (const auto& walk : walks0) dst.emplace_back(walk);
        for (const auto& walk : walks1) delete walk;

    } else if (walks1.size() > 1) {
        for (const auto& walk : walks1) dst.emplace_back(walk);
        for (const auto& walk : walks0) delete walk;

    } else {
        for (const auto& walk : walks) delete walk;
    }

    // maybe add restriction for walks so they don't have any external overlaps
}

const StringGraphNode* StringGraph::bubbleJuncture(StringGraphNode* rootNode) const {

    std::vector<StringGraphNode*> nodes;
    nodes.insert(nodes.end(), openedQueue_.begin(), openedQueue_.end());
    nodes.insert(nodes.end(), closedQueue_.begin(), closedQueue_.end());

    for (const auto& candidateNode : openedQueue_) {

        if (candidateNode->getVertex()->getId() == rootNode->getVertex()->getId()) {
            continue;
        }

        size_t walks = 0;

        for (const auto& node : nodes) {
            if (candidateNode->isInWalk(node)) ++walks;
        }

        if (walks > 1) {
            return candidateNode;
        }
    }

    return nullptr;
}

void StringGraph::extractBubbleWalks(std::vector<StringGraphWalk*>& dst, const Vertex* root,
    const StringGraphNode* junctureNode) const {

    std::vector<const StringGraphNode*> nodes;
    nodes.insert(nodes.end(), openedQueue_.begin(), openedQueue_.end());
    nodes.insert(nodes.end(), closedQueue_.begin(), closedQueue_.end());

    std::vector<const StringGraphNode*> junctureNodes;

    for (const auto& node : nodes) {
        const auto& temp = junctureNode->findInWalk(node);
        if (temp != nullptr) junctureNodes.emplace_back(temp);
    }

    nodes.clear();
    nodes.insert(nodes.end(), junctureNodes.begin(), junctureNodes.end());

    for (const auto* node : nodes) {

        std::vector<const Edge*> walkEdges;

        while (node->getParent() != nullptr) {
            walkEdges.emplace_back(node->getEdgeFromParent());
            node = node->getParent();
        }

        dst.emplace_back(new StringGraphWalk(root));

        for (auto it = walkEdges.rbegin(); it != walkEdges.rend(); ++it) {
            dst.back()->addEdge(*it);
        }
    }
}

bool StringGraph::popBubble(const std::vector<StringGraphWalk*> walks, int direction) {

    size_t selectedWalk;
    double selectedCoverage = 0;

    int overlapStart = std::numeric_limits<int>::max();
    int overlapEnd = std::numeric_limits<int>::max();

    size_t i = 0;
    for (const auto& walk : walks) {

        double coverage = 0;
        for (const auto& edge : walk->getEdges()) {
            coverage += edge->getB()->getCoverage();
        }

        if (coverage > selectedCoverage) {
            selectedWalk = i;
            selectedCoverage = coverage;
        }

        overlapStart = std::min(overlapStart, walk->getEdges().front()->getOverlap()->getLength());
        overlapEnd = std::min(overlapEnd, walk->getEdges().back()->getOverlap()->getLength());

        ++i;
    }

    std::vector<std::string> sequences;

    size_t j = 0;
    for (const auto& walk : walks) {

        const Vertex* root = walk->getEdges().front()->getA();
        const Vertex* juncture = walk->getEdges().back()->getB();

        if (j++ == selectedWalk) printf("!! ");

        std::string sequence;
        walk->extractSequence(sequence);

        int start, end;

        if (direction == 0) {
            // start     --------
            // end   -------
            // full  ------------
            // out      ------
            start = juncture->getLength() - overlapEnd;
            end = sequence.size() - (root->getLength() - overlapStart);

        } else {
            // start --------
            // end        -------
            // full  ------------
            // out      ------
            start = root->getLength() - overlapStart;
            end = sequence.size() - (juncture->getLength() - overlapEnd);
        }

        sequences.emplace_back(end > start ? sequence.substr(start, end - start) : std::string());
    }

    bool popped = false;

    for (size_t i = 0; i < sequences.size(); ++i) {

        if (i == selectedWalk) {
            continue;
        }

        int distance = editDistance(sequences[i], sequences[selectedWalk]);

        if (distance / (double) sequences[selectedWalk].size() < MAX_DIFFERENCE) {

            popped = true;

            // mark walk for removal

            const auto& edges = walks[i]->getEdges();

            // needed for walks which consist of only 1 edge
            const Vertex* root = edges.front()->getA();
            vertices_[verticesDict_.at(root->getId())]->markEdge(edges.front()->getId());

            for (size_t e = 0; e < edges.size() - 1; ++e) {

                Vertex* vertex = vertices_[verticesDict_.at(edges[e]->getB()->getId())];

                if (walks[selectedWalk]->containsVertex(vertex->getId())) {
                    continue;
                }

                vertex->mark();
                vertex->markEdges();

                for (const auto& edge : vertex->getEdgesB()) {
                    marked_.emplace_back(edge->oppositeVertex(vertex->getId())->getId());
                }

                for (const auto& edge : vertex->getEdgesE()) {
                    marked_.emplace_back(edge->oppositeVertex(vertex->getId())->getId());
                }
            }
        }
    }

    printf("popped = %d\n\n", popped);

    return popped;
}

void StringGraph::deleteMarked() {

    // remove marked edges which aare marked due to deletion of their opposite edges
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
    visited_.insert(edge->getB()->getId());
}

void StringGraphWalk::extractSequence(std::string& dst) const {

    bool prefix = !edges_.empty() && edges_.front()->getOverlap()->isUsingPrefix(start_->getId());

    // add start vertex
    dst = prefix ? std::string(start_->getSequence().rbegin(), start_->getSequence().rend()) :
        std::string(start_->getSequence());

    // types: 0 - normal, 1 - reverse complement
    auto getType = [](const Edge* edge, int id) -> int {
        if (edge->getOverlap()->getA() == id) return 0; // due to possible overlap types
        if (!edge->getOverlap()->isInnie()) return 0;
        return 1;
    };

    int prevType = getType(edges_.front(), edges_.front()->getA()->getId());

    printf("%d ", start_->getId());

    // add edge labels
    for (const auto& edge : edges_) {

        printf("-> %d ", edge->getB()->getId());

        int type = getType(edge, edge->getA()->getId());

        bool invert = type == prevType ? false : true;

        std::string label;
        if (invert) {
            edge->rkLabel(label);
        } else {
            edge->label(label);
        }

        dst += prefix ? std::string(label.rbegin(), label.rend()) : label;

        prevType = getType(edge, edge->getB()->getId()) ^ invert;
    }
    printf("\n");

    if (prefix) dst = std::string(dst.rbegin(), dst.rend());
}

bool StringGraphWalk::containsVertex(int id) const {
    return visited_.count(id) > 0;
}

// StringGraphNode

StringGraphNode::StringGraphNode(const Vertex* vertex, const Edge* edgeFromParent, const StringGraphNode* parent, int dir, int distance) :
    vertex_(vertex), edgeFromParent_(edgeFromParent), parent_(parent), direction_(dir) {

    if (parent_ == nullptr) {
        distance_ = 0;
    } else {
        distance_ = parent_->distance_ + distance;
    }
}

size_t StringGraphNode::expand(std::deque<StringGraphNode*>& queue) {

    const auto& edges = direction_ == 0 ? vertex_->getEdgesB() : vertex_->getEdgesE();

    for (const auto& edge : edges) {

        if (edge->isMarked()) {
            continue;
        }

        std::string label;
        edge->label(label);

        queue.emplace_back(new StringGraphNode(edge->getB(), edge, this,
            edge->getOverlap()->isInnie() ? (direction_ ^ 1) : direction_,
            label.size()));
    }

    return edges.size();
}

bool StringGraphNode::isInWalk(const StringGraphNode* node) const {

    if (node == nullptr) return false;
    if (node->getVertex()->getId() == this->getVertex()->getId() && node->getParent() != nullptr) return true;
    return this->isInWalk(node->getParent());
}

const StringGraphNode* StringGraphNode::findInWalk(const StringGraphNode* node) const {

    if (node == nullptr) return nullptr;
    if (node->getVertex()->getId() == this->getVertex()->getId()) return node;
    return this->findInWalk(node->getParent());
}

//*****************************************************************************
