/*
* StringGraph.cpp
*
* Created on: May 30, 2015
*     Author: rvaser
*/

#include "StringGraph.hpp"

//*****************************************************************************
// Edge

bool compareEdges(const Edge* left, const Edge* right) {

    if (left->getA()->getId() != right->getA()->getId()) {
        return left->getA()->getId() < right->getA()->getId();
    }

    if (left->getB()->getId() != right->getB()->getId()) {
        return left->getB()->getId() < right->getB()->getId();
    }

    return false;
}

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

const Vertex* Edge::oppositeVertex(int id) const {

    if (id == a_->getId()) return b_;
    if (id == b_->getId()) return a_;

    ASSERT(false, "Vertex", "wrong vertex id");
}

//*****************************************************************************



//*****************************************************************************
// Vertex

Vertex::Vertex(const Read* read, const StringGraph* graph) :
    read_(read), graph_(graph), marked_(false) {
}

bool Vertex::isTipCandidate() const {
    if (edgesB_.size() == 0 || edgesE_.size() == 0) return true;
    return false;
}

void Vertex::addEdge(Edge* edge) {

    if (edge->getOverlap()->isUsingSuffix(this->getId())) {
        edgesE_.emplace_back(edge);
    } else {
        edgesB_.emplace_back(edge);
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
        verticesDict_[read->getId()] = vertices_.size();
        vertices_.emplace_back(new Vertex(read, this));
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

    std::sort(edges_.begin(), edges_.end(), compareEdges);

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
                    marked_.push_back(edge->oppositeVertex(vertex->getId())->getId());
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

    size_t bubblesDir1 = 0;
    size_t bubblesDir2 = 0;
    size_t walksNum = 0;

    // size_t bubblesPoppedNum = 0;

    for (const auto& vertex : vertices_) {

        if (vertex->isMarked()) {
            continue;
        }

        std::vector<StringGraphWalk*> walks;
        findBubbleWalks(walks, vertex, 0);

        size_t temp = walks.size();

        findBubbleWalks(walks, vertex, 1);

        bubblesDir1 += temp != 0 ? 1 : 0;
        bubblesDir2 += walks.size() - temp != 0 ? 1 : 0;

        walksNum += walks.size();

        for (const auto& walk : walks) delete walk;
    }

    //if (bubblesPoppedNum > 0) {
    //    deleteMarked();
    //}

    fprintf(stderr, "[SG][bubble popping]: found %zu walks\n", walksNum);
    fprintf(stderr, "[SG][bubble popping]: found ( %zu , %zu ) bubbles\n", bubblesDir1, bubblesDir2);

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

            //fprintf(stderr, "[SG][Bubble popping]: found bubble(s) between %d and %d\n",
            //    root->getId(), junctureNode->getVertex()->getId());

            extractBubbleWalks(dst, root, junctureNode);
            break;
        }
    }

    for (const auto& it : nodes_) delete it;

    if (dst.size() < 2) {

        for (const auto& it : dst) delete it;
        std::vector<StringGraphWalk*>().swap(dst);

        return;
    }

    // check the orientation of last vertices of all walks (might be different due to innie overlaps)
}

const StringGraphNode* StringGraph::bubbleJuncture(StringGraphNode* rootNode) const {

    std::vector<StringGraphNode*> nodes;
    nodes.insert(nodes.end(), openedQueue_.begin(), openedQueue_.end());
    nodes.insert(nodes.end(), closedQueue_.begin(), closedQueue_.end());

    for (const auto& candidateNode : openedQueue_) {

        if (candidateNode->getVertex()->getId() == rootNode->getVertex()->getId()) {
            continue;
        }

        size_t branches = 0;

        for (const auto& node : nodes) {
            if (candidateNode->isInBranch(node)) ++branches;
        }

        if (branches > 1) {
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
        junctureNodes.emplace_back(node->findInBranch(junctureNode));
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

    // add edge labels
    for (const auto& edge : edges_) {

        std::string label;
        edge->label(label);

        dst += prefix ? std::string(label.rbegin(), label.rend()) : label;
    }

    if (prefix) dst = std::string(dst.rbegin(), dst.rend());
}

bool StringGraphWalk::containsVertex(int id) const {
    return visited_.count(id) > 0;
}

// StringGraphNode

StringGraphNode::StringGraphNode(const Vertex* vertex, const Edge* edgeFromParent, StringGraphNode* parent, int dir, int distance) :
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

        std::string label;
        edge->label(label);

        queue.emplace_back(new StringGraphNode(edge->getB(), edge, this,
            edge->getOverlap()->isInnie() ? (direction_ ^ 1) : direction_,
            label.size()));
    }

    return edges.size();
}

bool StringGraphNode::isInBranch(const StringGraphNode* node) const {

    if (node == nullptr) return false;
    if (node->getVertex()->getId() == this->getVertex()->getId() && node->getParent() != nullptr) return true;
    return this->isInBranch(node->getParent());
}

const StringGraphNode* StringGraphNode::findInBranch(const StringGraphNode* node) const {

    if (node == nullptr) return nullptr;
    if (node->getVertex()->getId() == this->getVertex()->getId()) return node;
    return this->findInBranch(node->getParent());
}

//*****************************************************************************
