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

class Edge;
class Vertex;
class StringGraph;
class StringGraphWalk;
class StringGraphNode;
class StringGraphComponent;
class Contig;

class Edge {
public:

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

    void mark() {
        marked_ = true;
    }

    bool isMarked() const {
        return marked_;
    }

    void label(std::string& dst) const;
    void rkLabel(std::string& dst) const;

    const Vertex* oppositeVertex(int id) const;

    friend class Vertex;
    friend class StringGraph;

private:

    int id_;
    const Vertex* a_;
    const Vertex* b_;
    const Overlap* overlap_;
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

    double getCoverage() const {
        return read_->getCoverage();
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

    bool isBubbleRootCandidate(int direction) const;

    void addEdge(Edge* edge);

    void markEdge(int id);

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

    size_t getNumVertices() const {
        return vertices_.size();
    }

    // minimal length of a vertex (read) to avoid trimming
    void trim();

    void popBubbles();

    // uses both trimming and bubble popping until no changes happen
    void simplify();

    void extractOverlaps(std::vector<Overlap*>& dst, bool view = true) const;

    void extractComponents(std::vector<StringGraphComponent*>& dst) const;

private:

    void findBubbleWalks(std::vector<StringGraphWalk*>& dst, const Vertex* root, int dir);

    const StringGraphNode* bubbleJuncture(StringGraphNode* rootNode) const;

    void extractBubbleWalks(std::vector<StringGraphWalk*>& dst, const Vertex* root,
        const StringGraphNode* junctureNode) const;

    bool popBubble(const std::vector<StringGraphWalk*>& walks, int direction);

    bool isValidWalk(int id, const std::vector<StringGraphWalk*>& walks) const;

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

    const Vertex* getStart() const {
        return start_;
    }

    const std::vector<const Edge*>& getEdges() const {
        return edges_;
    }

    void addEdge(const Edge* edge);

    void extractSequence(std::string& dst) const;

    bool containsVertex(int id) const;

    bool containsEdge(int id) const;

private:

    const Vertex* start_;
    std::vector<const Edge*> edges_;
    std::set<int> visitedVertices_;
    std::set<int> visitedEdges_;
};

// BFS wrapper for Vertex
class StringGraphNode {
public:

    StringGraphNode(const Vertex* vertex, const Edge* edgeFromParent,
        const StringGraphNode* parent, int dir, int distance);
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

    bool isInWalk(const StringGraphNode* node) const;

    const StringGraphNode* findInWalk(const StringGraphNode* node) const;

private:

    const Vertex* vertex_;
    const Edge* edgeFromParent_;
    const StringGraphNode* parent_;
    int direction_;
    int distance_;
};

// Connected component of the string graph
class StringGraphComponent {
public:

    StringGraphComponent(const std::set<int> vertexIds, const StringGraph* graph);
    ~StringGraphComponent();

    Contig* createContig();

    void extractSequence(std::string& dst);

private:

    void extractLongestWalk();

    std::vector<const Vertex*> vertices_;
    const StringGraph* graph_;
    StringGraphWalk* walk_;
};

class Contig {
public:

    // read id, type: normal 0 - rk 1, offset, lo, hi
    typedef std::tuple<int, int, int, int, int> Part;

    Contig() {}
    Contig(const StringGraphWalk* walk);
    ~Contig() {}

    const std::vector<Part>& getParts() const {
        return parts_;
    }

    void addPart(const Part& part) {
        parts_.emplace_back(part);
    }

private:

    std::vector<Part> parts_;
};