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

    openedQueue_.clear();
    closedQueue_.clear();
    nodes_.clear();

    // BFS search the string graph
    StringGraphNode* rootNode = new StringGraphNode(root, nullptr, nullptr, direction, 0);

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

        if (lastEdge->getOverlap()->isUsingSuffix(lastEdge->getDst()->getId())) {
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

    std::vector<std::string> sequences;

    for (const auto& walk : walks) {

        const Vertex* root = walk->getStart();
        const Vertex* juncture = walk->getEdges().back()->getDst();

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

        if (distance / (double) sequences[selectedWalk].size() < MAX_DIFFERENCE && isValidWalk(i, walks)) {

            // mark walk for removal

            const auto& edges = walks[i]->getEdges();

            // needed for walks which consist of only 1 edge ("trasitive" walks)
            if (edges.size() == 1) {

                const Vertex* start = edges.front()->getSrc();
                const Vertex* end = edges.front()->getDst();

                vertices_[verticesDict_.at(start->getId())]->markEdge(edges.front()->getId());

                marked_.emplace_back(start->getId());
                marked_.emplace_back(end->getId());
            }

            for (size_t e = 0; e < edges.size() - 1; ++e) {

                Vertex* vertex = vertices_[verticesDict_.at(edges[e]->getDst()->getId())];

                if (walks[selectedWalk]->containsVertex(vertex->getId())) {

                    if (!walks[selectedWalk]->containsEdge(edges[e]->getId())) {

                        Vertex* opposite = vertices_[verticesDict_.at(edges[e]->getSrc()->getId())];

                        opposite->markEdge(edges[e]->getId());

                        marked_.emplace_back(vertex->getId());
                        marked_.emplace_back(opposite->getId());

                        popped = true;
                    }

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

                popped = true;
            }
        }
    }

    return popped;
}

bool StringGraph::isValidWalk(int id, const std::vector<StringGraphWalk*>& walks) const {

    std::set<int> verticesIds;

    for (size_t i = 0; i < walks.size(); ++i) {

        const auto& edges = walks[i]->getEdges();

        verticesIds.insert(walks[i]->getStart()->getId());

        for (size_t j = 0; j < edges.size(); ++j) {
            verticesIds.insert(edges[j]->getDst()->getId());
        }
    }

    bool valid = true;

    const auto& edges = walks[id]->getEdges();

    for (size_t i = 0; i < edges.size() - 1; ++i) {

        const Vertex* vertex = edges[i]->getDst();

        const auto& edgesB = vertex->getEdgesB();
        for (const auto& edge : edgesB) {
            if (!edge->isMarked() && verticesIds.count(edge->getDst()->getId()) == 0) {
                valid = false;
                break;
            }
        }

        if (!valid) break;

        const auto& edgesE = vertex->getEdgesE();
        for (const auto& edge : edgesE) {
            if (!edge->isMarked() && verticesIds.count(edge->getDst()->getId()) == 0) {
                valid = false;
                break;
            }
        }

        if (!valid) break;
    }

    return valid;
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

size_t StringGraphNode::expand(std::deque<StringGraphNode*>& queue) {

    const auto& edges = direction_ == 0 ? vertex_->getEdgesB() : vertex_->getEdgesE();

    for (auto& edge : edges) {

        if (edge->isMarked()) {
            continue;
        }

        queue.emplace_back(new StringGraphNode(edge->getDst(), edge, this,
            edge->getOverlap()->isInnie() ? (direction_ ^ 1) : direction_,
            edge->labelLength()));
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

// StringGraphComponent

static int lengthRecursive(const Vertex* vertex, int direction, std::vector<bool>& visited, int branch,
    int maxBranch) {

    if (branch > maxBranch || visited[vertex->getId()]) {
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

static double expandVertex(std::vector<const Edge*>& dst, const Vertex* start, int direction, int maxId) {

    int totalLength = start->getLength();
    const Vertex* vertex = start;

    std::vector<bool> visitedVertices(maxId + 1, false);

    while (true) {

        if (visitedVertices[vertex->getId()]) {
            break;
        }

        visitedVertices[vertex->getId()] = true;

        const auto& edges = direction == 0 ? vertex->getEdgesB() : vertex->getEdgesE();

        if (edges.size() == 0) {
            break;
        }

        Edge* selectedEdge = edges.front();
        double selectedLength = 0;

        if (edges.size() > 1) {

            for (const auto& edge : edges) {

                const Vertex* next = edge->getDst();

                if (visitedVertices[next->getId()]) {
                    continue;
                }

                int length = lengthRecursive(next, edge->getOverlap()->isInnie() ? (direction ^ 1) :
                    direction, visitedVertices, 0, MAX_BRANCHES);

                if (length > selectedLength) {
                    selectedEdge = edge;
                    selectedLength = length;
                }
            }
        }

        dst.emplace_back(selectedEdge);
        vertex = selectedEdge->getDst();

        totalLength += selectedEdge->labelLength();

        if (selectedEdge->getOverlap()->isInnie()) {
            direction ^= 1;
        }
    }

    return totalLength;
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
        int length = expandVertex(edges, start, direction, maxId);

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

// StringGraphComponent

Contig* StringGraphComponent::createContig() {

    if (walk_ == nullptr) extractLongestWalk();
    if (walk_ != nullptr) return new Contig(walk_);
    return nullptr;
}

//*****************************************************************************
//
//*****************************************************************************
// ContigExtractor

template <>
struct std::hash<std::pair<int, bool>> {
public:
    size_t operator()(std::pair<int, bool> x) const throw() {
        return x.first ^ x.second;
    }
};

Contig* ContigExtractor::extractContig() {

    int max_id = 0;

    for (const auto& v: component_->vertices_) {
        max_id = std::max(max_id, v->getId());
    }

    for (int direction = 0; direction <= 1; ++direction) {

        for (const auto& vertex : component_->vertices_) {

            if ((direction == 0 && vertex->getEdgesB().size() == 1 && vertex->getEdgesE().size() == 0) ||
                (direction == 1 && vertex->getEdgesE().size() == 1 && vertex->getEdgesB().size() == 0)) {

                std::unordered_map<std::pair<int, bool>, int> cache;
                std::vector<bool> visited(max_id + 1);

                for (const auto& v: component_->vertices_) {
                    fprintf(stdout, "+ %d%c %d\n",
                        v->getId(),
                        direction == 0 ? 'B' : 'E',
                        longestPath(v->getId(), direction, cache, visited)
                    );
                }
            }
        }
    }

    return nullptr;
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

//*****************************************************************************
