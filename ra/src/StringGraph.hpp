/*!
 * @file StringGraph.hpp
 *
 * @brief StringGraph and other classes header file
 * @details Code was rewritten and modified from:
 *    Github: https://github.com/mariokostelac/croler \n
 *    Github: https://github.com/jts/sga (trimming and bubble popping) \n
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 30, 2015
 */

#pragma once

#include "Globals.hpp"
#include "Read.hpp"
#include "Overlap.hpp"
#include "CommonHeaders.hpp"

class Edge;
class Vertex;
class StringGraph;
class StringGraphWalk;
class StringGraphNode;
class StringGraphComponent;

typedef std::map<int, Vertex*> VerticesSet;

/*!
 * @brief Edge class
 * @details Edge encapsulates Overlap in StringGraph.
 */
class Edge {
public:

    /*!
     * @brief Edgeructor
     * @details Creates an Edge object from a read and an overlap.
     *
     * @param [in] id edge identifier
     * @param [in] readId read identifier to determine edge direction
     * @param [in] overlap Overlap object pointer
     * @param [in] graph Graph object pointer the edge is in
     */
    Edge(uint32_t id, uint32_t readId, Overlap* overlap, StringGraph* graph);

    /*!
     * @brief Edge destructor
     */
    ~Edge() {};

    /*!
     * @brief Getter for identifier
     * @return identifier
     */
    uint32_t getId() {
        return id_;
    }

    /*!
     * @brief Getter for start vertex
     * @return start vertex
     */
    Vertex* getSrc() {
        return src_;
    }

    /*!
     * @brief Getter for end vertex
     * @return end vertex
     */
    Vertex* getDst() {
        return dst_;
    }

    /*!
     * @brief Getter for overlap
     * @return overlap
     */
    Overlap* getOverlap() {
        return overlap_;
    }

    /*!
     * @brief Method for edge marking
     * @details Marks this edge for removal from graph
     */
    void mark() {
        marked_ = true;
    }

    /*!
     * @brief Method for removal check
     * @details Checks if this edge is marked for removal from graph.
     *
     * @return true if this edge is marked
     */
    bool isMarked() {
        return marked_;
    }

    /*!
     * @brief Method for label extraction
     * @details Method creates the edge label
     *
     * @param [out] dst label string
     */
    void label(std::string& dst);

    int labelLength();

    /*!
     * @brief Method for reverse complement label extraction
     * @details Method calls label and reverse complements it.
     *
     * @param [out] dst rk label string
     */
    void rkLabel(std::string& dst);

    /*!
     * @brief Getter for opposite vertex on edge
     * @details Method retuns the opposite vertex then the one given.
     *
     * @param [in] id vertex identifier
     * @return opposite vertex
     */
    Vertex* oppositeVertex(uint32_t id);

    // TODO
    Edge* pair();

    friend class Vertex;
    friend class StringGraph;

private:

    uint32_t id_;
    Vertex* src_;
    Vertex* dst_;
    Overlap* overlap_;
    Edge* pair_;
    StringGraph* graph_;
    bool marked_;
    int labelLength_;
};

/*!
 * @brief Vertex class
 * @details Vertex encapsulates Read in StringGraph.
 */
class Vertex {
public:

    /*!
     * @brief Vertexructor
     * @details Creates an Vertex object from a read.
     *
     * @param [in] id vertex identifier
     * @param [in] read Read object pointer
     * @param [in] graph Graph object pointer the edge is in
     */
    Vertex(uint32_t id, Read* read, StringGraph* graph);

    /*!
     * @brief Vertex destructor
     */
    ~Vertex() {};

    /*!
     * @brief Getter for identifier
     * @return identifier
     */
    uint32_t getId() {
        return id_;
    }

    /*!
     * @brief Getter for read identifier
     * @return read identifier
     */
    uint32_t getReadId() {
        return read_->id();
    }

    /*!
     * @brief Getter for read legnth
     * @return length
     */
    int getLength() {
        return read_->length();
    }

    /*!
     * @brief Getter for read sequence
     * @return sequence
     */
    const std::string& getSequence() {
        return read_->sequence();
    }

    /*!
     * @brief Getter for read reverse complement
     * @return reverse complement
     */
    const std::string& getReverseComplement() {
        return read_->reverse_complement();
    }

    /*!
     * @brief Getter for read coverage
     * @return coverage
     */
    double getCoverage() {
        return read_->coverage();
    }

    /*!
     * @brief Getter for edges where the read Begining is in the overlap
     * @return list of edges
     */
    std::list<Edge*>& getEdgesB() {
        return edgesB_;
    }

    /*!
     * @brief Getter for edges where the read End is in the overlap
     * @return list of edges
     */
    std::list<Edge*>& getEdgesE() {
        return edgesE_;
    }

    /*!
     * @brief Method for vertex marking
     * @details Marks this vertex for removal from graph.
     */
    void mark() {
        marked_ = true;
    }

    /*!
     * @brief Method for removal check
     * @details Checks if this vertex is marked for removal from graph.
     *
     * @return true if this vertex is marked
     */
    bool isMarked() {
        return marked_;
    }

    /*!
     * @brief Method for tip candidate check
     * @details Checks if this vertex is a tip candidate, i.e. it has
     * only edges in one of its lists.
     *
     * @return true if this vertex is a tip candidate
     */
    bool isTipCandidate();

    /*!
     * @brief Method for bubble root candidate check
     * @details Checks if this vertex is a bubble root candidate in given diretion,
     * i.e. it has 2 or more edges in the directions lists.
     *
     * @return true if this vertex is a bubble root candidate
     */
    bool isBubbleRootCandidate(int direction);

    /*!
     * @brief Method for adding new edges
     * @details Method adds edge to one of the list depending on the type.
     *
     * @param [in] edge Edge object pointer
     */
    void addEdge(Edge* edge);

    /*!
     * @brief Method for edge marking
     * @details Marks all edges in both lists and their pairs for removal.
     */
    void markEdges();

    /*!
     * @brief Method for edge removal
     * @details Method removes marked edges in both lists but does not delete them from the graph!
     *
     * @param [int] propagate propagate call to all vertices adjacent with removed edges.
     */
    void removeMarkedEdges(bool propagate = true);


    /*!
     * @brief Method for testing if edge uses vertex's read prefix
     * @details Method for testing if edge uses vertex's read prefix
     *
     * @return true if e uses vertex's read prefix
     */
    bool isBeginEdge(Edge* e);

    // TODO
    Edge* bestEdge(bool use_end);

private:

    uint32_t id_;
    Read* read_;
    StringGraph* graph_;
    bool marked_;

    std::list<Edge*> edgesB_;
    std::list<Edge*> edgesE_;
};

/*!
 * @brief StringGraph class
 * @details Consists of vertices and edges between them.
 */
class StringGraph {
public:

    /*!
     * @brief StringGraphructor
     * @details Creates a StringGraph object from reads and overlaps between them.
     * Each Read becomes a Vertex and from each Overlap two Edges are created,
     * one from A to B and one from B to A (A and B are reads in Overlap).
     *
     * @param [in] reads vector of Read object pointers
     * @param [in] overlaps vector of Overlap object pointers
     */
    StringGraph(std::vector<Read*>& reads, std::vector<Overlap*>& overlaps);

    /*!
     * @brief StrigGraphructor
     */
    ~StringGraph();

    /*!
     * @brief Getter for a vertex
     * @details Returns vertex with given identifier
     *
     * @param [in] id vertex identifier
     * @return vertex
     */
    Vertex* getVertex(int id) {
        if (!vertices_.count(id)) {
          return nullptr;
        }

        return vertices_.at(id);
    }

    /*!
     * @brief Getter for number of vertices in graph
     * @return number of vertices
     */
    size_t getNumVertices() {
        return vertices_.size();
    }

    /*!
     * @brief Getter for number of edges in graph
     * @return number of edges
     */
    size_t getNumEdges() {
        return edges_.size();
    }

    /*!
     * @brief Method for graph simplification
     * @details Method removes and deletes all tips and disconnected
     * vertices from the graph
     */
    void trim();

    /*!
     * @brief Method for graph simplification
     * @details Method finds all bubbles in graph and if possible removes
     * all paths (removes and deletes all vertices an their edges) from the
     * bubble but the best one.
     * TODO
     */
    uint32_t popBubbles();

    /*!
     * @brief Method for graph simplification
     * @details Method calls both trimming and bubble popping in an alternating
     * fashion until no changes occur in graph.
     */
    void simplify();

    // TODO
    int reduceToBOG();

    /*!
     * @brief Method for overlap extracion
     * @details Method returns all overlaps present in graph.
     *
     * @param [out] dst vector of Overlap object pointers
     * @param [in] view if true Overlap objects are not cloned to dst
     */
    void extractOverlaps(std::vector<Overlap*>& dst, bool view = true);

    /*!
     * @brief Methd for graph component extraction
     * @details Method returns all graph components (component is a set of
     * vertices that are connected together by edges).
     *
     * @param [out] dst vector of StringGraphComponent object pointers
     */
    void extractComponents(std::vector<StringGraphComponent*>& dst);

    int extract_unitigs(std::vector<StringGraphWalk*>* walks);

    /*!
     * @brief Method for deletion of vertices and edges
     * @details Method first removes edges from lists of each vertex in marked_.
     * Afterwards it deletes all marked edges and vertices from the graph.
     */
    void delete_marked();

private:

    void delete_marked_edges();
    void delete_marked_vertices();

    /**
     * TODO
     */
    int mark_unitig(std::vector<Edge*>* dst_edges, std::vector<int>* unitig_id,
        int id, Vertex* start, int direction);

    /*!
     * @brief Method for bubble search
     * @details A breadth first search is executed from root until a juncture vertex is found.
     * If a juncutre exists backtracked paths from juncture to root are added to the bubble.
     * The orientation of juncture vertices can be different on more paths so the bubble is
     * split into two and the one with more than 1 path is returned.
     *
     * @param [out] dst vector of bubble paths
     * @param [in] root vertex from which the BFS is executed
     * @param [in] direction direction the BFS is executed (Begin or End)
     * TODO
     */
    uint32_t popBubblesStartingAt(Vertex* root, int direction);

    /*!
     * @brief Method for bubble popping
     * @details Method tries to remove all paths from bubble but the one with the highest
     * coverage. Firstraint is that paths must share similar sequences (at least 95%).
     * The secondraint is that if a path that is a candidate for removal has extrenal
     * edges (edges not pointingt into the bubble) it won't be removed.
     *
     * @param [in] walks bubble consisting of 2 or more paths (walks)
     * @param [in] direction direction the paths in bubble are pointing (Begin or End) needed
     * for sequence extraction
     */
    bool popBubble(std::vector<StringGraphWalk*>& walks, uint32_t juncture_id, int direction);

    std::vector<Overlap*>* overlaps_;

    std::vector<Edge*> edges_;
    VerticesSet vertices_;
};

/*!
 * @brief StringGraphWalk class
 * @details Used to store paths in the StringGraph
 */
class StringGraphWalk {
public:

    /*!
     * @brief StringGraphWalkructir
     * @details Creates a StringGraphWalk object from the starting vertex
     *
     * @param [in] start start vertex
     */
    StringGraphWalk(Vertex* start);

    /*!
     * @brief StringGraphWalk destructor
     */
    ~StringGraphWalk() {};

    /*!
     * @brief Getter for start vertex
     * @return start vertex
     */
    Vertex* getStart() {
        return start_;
    }

    /*!
     * @brief Getter for edges
     * @return vector of Edge object pointers
     */
    std::vector<Edge*>& getEdges() {
        return edges_;
    }

    /*!
     * @brief Method for path extension
     * @details Adds edge to end of path
     *
     * @param [in] edge edge to be added to the path
     */
    void addEdge(Edge* edge);

    /*!
     * @brief Method for sequence extraction
     * @details Method returns the sequence extracted from the path.
     * It equals the sequence from start vertex plus all edge labels.
     *
     * @param [out] dst sequence string
     */
    void extractSequence(std::string& dst);

    /*!
     * @brief Method for vertices extraction
     * @details Method appends vertices ids extracted from the path.
     *
     * @param [out] dst vertices
     */
    void extractVertices(std::vector<Vertex*>& dst);

    /*!
     * @brief Method for vertex check
     * @details Cheks whether a vertex with given identifier exists in path.
     *
     * @param [in] id vertex identifier
     * @return true if the vertex is in path
     */
    bool containsVertex(int id);

    /*!
     * @brief Method for edge check
     * @details Cheks whether a edge with given identifier exists in path.
     *
     * @param [in] id edge identifier
     * @return true if the edge is in path
     */
    bool containsEdge(int id);

private:

    Vertex* start_;
    std::vector<Edge*> edges_;
    std::set<int> visitedVertices_;
    std::set<int> visitedEdges_;
};

/*!
 * @brief StringGraphNode class
 * @details Wrapper for Vertex needed for BFS.
 */
class StringGraphNode {
public:

    /*!
     * @brief StrinGraphNodeructor
     * @details Creates a StrinGraphNode object from vertex, parent vertex,
     * edge from parent vertex, direction for exapnsion and distance traveled
     * so far in path.
     *
     * @param [in] vertex Vertex object pointer
     * @param [in] edgeFromParent Edge object pointer
     * @param [in] parent parent Vertex object pointer
     * @param [in] direction direction for expansion (Begin or End)
     * @param [in] distance distance traveled so far in path
     */
    StringGraphNode(Vertex* vertex, Edge* edgeFromParent,
        StringGraphNode* parent, int direction, int distance);

    /*!
     * @brief StringGraphNode desctructor
     */
    ~StringGraphNode() {};

    /*!
     * @brief Getter for vertex
     * @return vertex
     */
    Vertex* getVertex() {
        return vertex_;
    }

    /*!
     * @brief Getter for edge from parent
     * @return edge from parent
     */
    Edge* getEdgeFromParent() {
        return edgeFromParent_;
    }

    /*!
     * @brief Getter for parent vertex
     * @return parent vertex
     */
    StringGraphNode* getParent() {
        return parent_;
    }

    /*!
     * @brief Getter for distance
     * @return distance
     */
    int getDistance() {
        return distance_;
    }

    /*!
     * @brief Method for path expansion
     * @details Method expands this vertex in direction and fills the input deque
     *
     * @param [in] vector of StringGraphNode object pointers
     * @return number of StringGraphNodes added to queue
     */
    size_t expand(std::vector<StringGraphNode*>& queue);

    /*!
     * @brief Method for walk check
     * @details Method follows the edges from parent until root node is reached. If the
     * given node is found along this path function returns true.
     *
     * @param [in] node StingGraphNode object pointer
     * @return true if node is in path to root
     */
    bool isInWalk(StringGraphNode* node);

    /*!
     * @brief Method for walk search
     * @details Method follows the edges from parent until root node is reached. If the
     * given node is found along this path it is returned.
     *
     * @param [in] node StingGraphNode object pointer
     * @return node in path (if not found nullptr is returned)
     */
    StringGraphNode* findInWalk(StringGraphNode* node);

    /*!
     * @brief Method returns preceeding StringGraphNode* with given vertexId.
     * @details Method returns preceeding StringGraphNode* with given vertexId.
     *
     * @param [in] vertexId vertexId
     * @return preceeding node with given vertexId, nullptr if does not exist such node.
     */
    StringGraphNode* rewindedTo(uint32_t vertexId);

    /*!
     * @brief Method creates StringGraphWalk from this node and it's predecessors.
     * @details Method creates StringGraphWalk from this node and it's predecessors.
     *
     * @return StringGraphWalk from this node and it's predecessors.
     */
    StringGraphWalk* getWalk();

private:

    Vertex* vertex_;
    Edge* edgeFromParent_;
    StringGraphNode* parent_;
    int direction_;
    int distance_;
};

/*!
 * @brief StringGraphComponent class
 * @details Holds a connected component of the string graph
 */
class StringGraphComponent {
public:

    /*!
     * @brief StringGraphComponentructor
     * @details Creates a StringGraphComponent object from graph by picking out
     * vertices with given identifiers.
     *
     * @param [in] vertexIds vertex identifiers
     * @param [in] graph StringGraph object pointer
     */
    StringGraphComponent(std::set<int> vertexIds, StringGraph* graph);

    /*!
     * @brief StringGraphComponent destructor
     */
    ~StringGraphComponent();

    /*!
     * @brief Method for sequence extraction
     * @details Calls extractLongestWalk if nullptr is store in walk_ and extracts
     * the sequence from walk_.
     *
     * @param [out] dst sequence string if a walk is found
     */
    void extractSequence(std::string& dst);

    /*
     * TODO
     */
    StringGraphWalk* longestWalk();

    std::vector<Vertex*>& vertices() {
      return vertices_;
    }

private:

    void extractLongestWalk();

    std::vector<Vertex*> vertices_;
    StringGraph* graph_;
    StringGraphWalk* walk_;
};
