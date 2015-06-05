/*
* StringGraph.hpp
*
* Created on: May 30, 2015
*     Author: rvaser
*
* Code was rewritten and modified from:
*     Github: https://github.com/mariokostelac/croler (private repository)
*/

#pragma once

#include "Read.hpp"
#include "Overlap.hpp"
#include "CommonHeaders.hpp"

//#include "edlib/edlib.h"

class Vertex;
class StringGraph;
class StringGraphWalk;
class StringGraphNode;

class Edge {
public:

    enum Direction { A_TO_B, B_TO_A };

    Edge(int id, int readId, const Overlap* overlap, const StringGraph* graph);
    ~Edge() {};

    int getId() const {
        return id_;
    }

    const Vertex* getA() const {
        return a_;
    }

    const Vertex* getB() const {
        return b_;
    }

    const Overlap* getOverlap() const {
        return overlap_;
    }

    const Direction& getDirection() const {
        return direction_;
    }

    void mark() {
        marked_ = true;
    }

    bool isMarked() const {
        return marked_;
    }

    void label(std::string& dst) const;

    const Vertex* oppositeVertex(int id) const;

    friend class Vertex;
    friend class StringGraph;

private:

    int id_;
    const Vertex* a_;
    const Vertex* b_;
    const Overlap* overlap_;
    Direction direction_;
    Edge* opposite_;
    const StringGraph* graph_;
    bool marked_;
};

class Vertex {
public:

    Vertex(int id, const Read* read, const StringGraph* graph);
    ~Vertex() {};

    int getId() const {
        return id_;
    }

    int getLength() const {
        return read_->getLength();
    }

    const std::string& getSequence() const {
        return read_->getSequence();
    }

    const std::string& getReverseComplement() const {
        return read_->getReverseComplement();
    }

    const std::list<Edge*>& getEdgesB() const {
        return edgesB_;
    }

    const std::list<Edge*>& getEdgesE() const {
        return edgesE_;
    }

    void mark() {
        marked_ = true;
    }

    bool isMarked() const {
        return marked_;
    }

    bool isTipCandidate() const;

    void addEdge(Edge* edge);

    void markEdges();

    void removeMarkedEdges();

private:

    int id_;
    const Read* read_;
    const StringGraph* graph_;
    bool marked_;

    std::list<Edge*> edgesB_;
    std::list<Edge*> edgesE_;
};

class StringGraph {
public:

    StringGraph(const std::vector<Read*>& reads, const std::vector<Overlap*>& overlaps);
    ~StringGraph();

    const Vertex* getVertex(int id) const {
        return vertices_[verticesDict_.at(id)];
    }

    // minimal length of a vertex (read) to avoid trimming
    void trim(int threshold = 100000);

    void popBubbles();

    void extractOverlaps(std::vector<Overlap*>& dst, bool view = true) const;

private:

    static const size_t MAX_NODES = 500;
    static const int MAX_DISTANCE = 1000;

    void findBubbleWalks(std::vector<StringGraphWalk*>& dst, const Vertex* root, int dir);

    const StringGraphNode* bubbleJuncture(StringGraphNode* rootNode) const;

    void extractBubbleWalks(std::vector<StringGraphWalk*>& dst, const Vertex* root,
        const StringGraphNode* junctureNode) const;

    void deleteMarked();

    const std::vector<Overlap*>* overlaps_;

    std::vector<Vertex*> vertices_;
    std::vector<Edge*> edges_;
    std::map<int, int> verticesDict_;

    // helper for edge removal (contains indices of vertices which have marked edges)
    std::vector<int> marked_;

    // queues for bubble popping
    std::deque<StringGraphNode*> openedQueue_;
    std::deque<StringGraphNode*> closedQueue_;
    // helper for bubble popping
    std::vector<StringGraphNode*> nodes_;
};

class StringGraphWalk {
public:

    StringGraphWalk(const Vertex* start);
    ~StringGraphWalk() {};

    const std::vector<const Edge*>& getEdges() const {
        return edges_;
    }

    void addEdge(const Edge* edge);

    void extractSequence(std::string& dst) const;

    bool containsVertex(int id) const;

private:

    const Vertex* start_;
    std::vector<const Edge*> edges_;
    std::set<int> visited_; // visited vertices
};

// BFS wrapper for Vertex
class StringGraphNode {
public:

    StringGraphNode(const Vertex* vertex, const Edge* edgeFromParent, StringGraphNode* parent,
        int dir, int distance);
    ~StringGraphNode() {};

    const Vertex* getVertex() const {
        return vertex_;
    }

    const Edge* getEdgeFromParent() const {
        return edgeFromParent_;
    }

    const StringGraphNode* getParent() const {
        return parent_;
    }

    int getDistance() const {
        return distance_;
    }

    size_t expand(std::deque<StringGraphNode*>& queue);

    bool isInBranch(const StringGraphNode* node) const;

    const StringGraphNode* findInBranch(const StringGraphNode* node) const;

private:

    const Vertex* vertex_;
    const Edge* edgeFromParent_;
    StringGraphNode* parent_;
    int direction_;
    int distance_;
};
