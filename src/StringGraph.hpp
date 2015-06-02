/*
* StringGraph.hpp
*
* Created on: May 30, 2015
*     Author: rvaser
*
* Code was rewritten from:
*     https://github.com/mariokostelac/croler (private repository)
*/

#pragma once

#include "Read.hpp"
#include "Overlap.hpp"
#include "CommonHeaders.hpp"

class Vertex;
class StringGraph;

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

    Vertex(const Read* read, const StringGraph* graph);
    ~Vertex() {};

    int getId() const {
        return read_->getId();
    }

    int getLength() const {
        return read_->getLength();
    }

    size_t getNumEdges() const {
        return numEdges_;
    }

    const std::vector<Edge*>& getEdgesB() const {
        return edgesB_;
    }

    const std::vector<Edge*>& getEdgesE() const {
        return edgesE_;
    }

    /*const std::vector<Edge*>& getEdgesDirA() const {
        return edgesDirA_;
    }

    const std::vector<Edge*>& getEdgesDirB() const {
        return edgesDirB_;
    }*/

    void mark() {
        marked_ = true;
    }

    bool isMarked() const {
        return marked_;
    }

    bool isTip() const;

    void addEdge(Edge* edge);

    void markEdges();

private:

    const Read* read_;
    const StringGraph* graph_;
    size_t numEdges_;
    bool marked_;

    std::vector<Edge*> edgesB_;
    std::vector<Edge*> edgesE_;
    // std::vector<Edge*> edgesDirA_; // A to B
    // std::vector<Edge*> edgesDirB_; // B to A
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

    void deleteMarked();

    const std::vector<Overlap*>* overlaps_;

    std::vector<Vertex*> vertices_;
    std::vector<Edge*> edges_;
    std::map<int, int> verticesDict_;
};
