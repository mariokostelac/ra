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

const Vertex* Edge::oppositeVertex(int id) const {

    if (id == a_->getId()) return b_;
    if (id == b_->getId()) return a_;

    ASSERT(false, "Vertex", "wrong vertex id");
}

//*****************************************************************************



//*****************************************************************************
// Vertex

Vertex::Vertex(const Read* read, const StringGraph* graph) :
    read_(read), graph_(graph), numEdges_(0), marked_(false) {
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

bool Vertex::isTip() const {
    if (edgesB_.size() == 0 || edgesE_.size() == 0) return true;
    return false;
}

void Vertex::addEdge(Edge* edge) {

    ++numEdges_;

    if (edge->getOverlap()->isUsingSuffix(this->getId())) {
        edgesE_.emplace_back(edge);
    } else {
        edgesB_.emplace_back(edge);
    }

    /*if (edge->getDirection() == Edge::Direction::A_TO_B) {
        edgesDirA_.emplace_back(edge);
    } else {
        edgesDirB_.emplace_back(edge);
    }*/
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
        if (vertex->isTip()) {

            const auto& edges = vertex->getEdgesB().size() == 0 ? vertex->getEdgesE() :
                vertex->getEdgesB();

            for (const auto& edge : edges) {

                // check if opposite vertex has other edges similar as this one

                const auto& opposite = edge->oppositeVertex(vertex->getId());

                const auto& oppositeEdges = edge->getOverlap()->isUsingSuffix(opposite->getId()) ?
                    opposite->getEdgesE() : opposite->getEdgesB();

                size_t notMarked = 0;

                for (const auto& oedge : oppositeEdges) {
                    if (!oedge->isMarked() && !oedge->oppositeVertex(opposite->getId())->isTip()) {
                        ++notMarked;
                    }
                }

                if (notMarked > 1) {
                    vertex->mark();
                    vertex->markEdges();
                    ++tipsNum;

                    break;
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

void StringGraph::extractOverlaps(std::vector<Overlap*>& dst, bool view) const {

    std::vector<bool> duplicates(overlaps_->size() * 2, false);

    dst.reserve(edges_.size() / 2);

    for (const auto& edge : edges_) {

        if (duplicates[edge->getId()]) continue;

        dst.push_back(view ? (*overlaps_)[edge->getId() / 2] :
            (*overlaps_)[edge->getId() / 2]->clone());

        duplicates[edge->opposite_->getId()] = true;
    }
}

void StringGraph::deleteMarked() {

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

//*****************************************************************************
