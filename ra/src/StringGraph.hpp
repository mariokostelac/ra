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
#include "DovetailOverlap.hpp"
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
 * @details Edge encapsulates DovetailOverlap in StringGraph.
 */
class Edge {
public:

    /*!
     * @brief Edge constructor
     * @details Creates an Edge object from a read and an overlap.
     *
     * @param [in] id edge identifier
     * @param [in] readId read identifier to determine edge direction
     * @param [in] overlap DovetailOverlap object pointer
     * @param [in] graph Graph object pointer the edge is in
     */
    Edge(int id, int readId, const DovetailOverlap* overlap, const StringGraph* graph);

    /*!
     * @brief Edge destructor
     */
    ~Edge() {};

    /*!
     * @brief Getter for identifier
     * @return identifier
     */
    int getId() const {
        return id_;
    }

    /*!
     * @brief Getter for start vertex
     * @return start vertex
     */
    const Vertex* getSrc() const {
        return src_;
    }

    /*!
     * @brief Getter for end vertex
     * @return end vertex
     */
    const Vertex* getDst() const {
        return dst_;
    }

    /*!
     * @brief Getter for overlap
     * @return overlap
     */
    const DovetailOverlap* getOverlap() const {
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
    bool isMarked() const {
        return marked_;
    }

    /*!
     * @brief Method for label extraction
     * @details Method creates the edge label
     *
     * @param [out] dst label string
     */
    void label(std::string& dst) const;

    int labelLength();

    /*!
     * @brief Method for reverse complement label extraction
     * @details Method calls label and reverse complements it.
     *
     * @param [out] dst rk label string
     */
    void rkLabel(std::string& dst) const;

    /*!
     * @brief Getter for opposite vertex on edge
     * @details Method retuns the opposite vertex then the one given.
     *
     * @param [in] id vertex identifier
     * @return opposite vertex
     */
    const Vertex* oppositeVertex(int id) const;

    // TODO
    Edge* pair() const;

    friend class Vertex;
    friend class StringGraph;

private:

    int id_;
    const Vertex* src_;
    const Vertex* dst_;
    const DovetailOverlap* overlap_;
    Edge* pair_;
    const StringGraph* graph_;
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
     * @brief Vertex constructor
     * @details Creates an Vertex object from a read.
     *
     * @param [in] id vertex identifier
     * @param [in] read Read object pointer
     * @param [in] graph Graph object pointer the edge is in
     */
    Vertex(int id, const Read* read, const StringGraph* graph);

    /*!
     * @brief Vertex destructor
     */
    ~Vertex() {};

    /*!
     * @brief Getter for identifier
     * @return identifier
     */
    int getId() const {
        return id_;
    }

    /*!
     * @brief Getter for read identifier
     * @return read identifier
     */
    int getReadId() const {
        return read_->getId();
    }

    /*!
     * @brief Getter for read legnth
     * @return length
     */
    int getLength() const {
        return read_->getLength();
    }

    /*!
     * @brief Getter for read sequence
     * @return sequence
     */
    const std::string& getSequence() const {
        return read_->getSequence();
    }

    /*!
     * @brief Getter for read reverse complement
     * @return reverse complement
     */
    const std::string& getReverseComplement() const {
        return read_->getReverseComplement();
    }

    /*!
     * @brief Getter for read coverage
     * @return coverage
     */
    double getCoverage() const {
        return read_->getCoverage();
    }

    /*!
     * @brief Getter for edges where the read Begining is in the overlap
     * @return list of edges
     */
    const std::list<Edge*>& getEdgesB() const {
        return edgesB_;
    }

    /*!
     * @brief Getter for edges where the read End is in the overlap
     * @return list of edges
     */
    const std::list<Edge*>& getEdgesE() const {
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
    bool isMarked() const {
        return marked_;
    }

    /*!
     * @brief Method for tip candidate check
     * @details Checks if this vertex is a tip candidate, i.e. it has
     * only edges in one of its lists.
     *
     * @return true if this vertex is a tip candidate
     */
    bool isTipCandidate() const;

    /*!
     * @brief Method for bubble root candidate check
     * @details Checks if this vertex is a bubble root candidate in given diretion,
     * i.e. it has 2 or more edges in the directions lists.
     *
     * @return true if this vertex is a bubble root candidate
     */
    bool isBubbleRootCandidate(int direction) const;

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
    void removeMarkedEdges(const bool propagate = true);


    /*!
     * @brief Method for testing if edge uses vertex's read prefix
     * @details Method for testing if edge uses vertex's read prefix
     *
     * @return true if e uses vertex's read prefix
     */
    const bool isBeginEdge(const Edge* e) const;

    // TODO
    const Edge* bestEdge(const bool use_end) const;

private:

    int id_;
    const Read* read_;
    const StringGraph* graph_;
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
     * @brief StringGraph constructor
     * @details Creates a StringGraph object from reads and overlaps between them.
     * Each Read becomes a Vertex and from each DovetailOverlap two Edges are created,
     * one from A to B and one from B to A (A and B are reads in DovetailOverlap).
     *
     * @param [in] reads vector of Read object pointers
     * @param [in] overlaps vector of DovetailOverlap object pointers
     */
    StringGraph(const std::vector<Read*>& reads, const std::vector<DovetailOverlap*>& overlaps);

    /*!
     * @brief StrigGraph constructor
     */
    ~StringGraph();

    /*!
     * @brief Getter for a vertex
     * @details Returns vertex with given identifier
     *
     * @param [in] id vertex identifier
     * @return vertex
     */
    const Vertex* getVertex(int id) const {
        if (!vertices_.count(id)) {
          return nullptr;
        }

        return vertices_.at(id);
    }

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
    size_t getNumVertices() const {
        return vertices_.size();
    }

    /*!
     * @brief Getter for number of edges in graph
     * @return number of edges
     */
    size_t getNumEdges() const {
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
     * @param [out] dst vector of DovetailOverlap object pointers
     * @param [in] view if true DovetailOverlap objects are not cloned to dst
     */
    void extractOverlaps(std::vector<Overlap*>& dst, bool view = true) const;

    /*!
     * @brief Methd for graph component extraction
     * @details Method returns all graph components (component is a set of
     * vertices that are connected together by edges). 
     *
     * @param [out] dst vector of StringGraphComponent object pointers
     */
    void extractComponents(std::vector<StringGraphComponent*>& dst) const;

    int extract_unitigs(std::vector<StringGraphWalk*>* walks) const;

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
        const int id, const Vertex* start, const int direction) const;

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
    uint32_t popBubblesStartingAt(const Vertex* root, int direction);

    /*!
     * @brief Method for bubble popping
     * @details Method tries to remove all paths from bubble but the one with the highest
     * coverage. First constraint is that paths must share similar sequences (at least 95%).
     * The second constraint is that if a path that is a candidate for removal has extrenal
     * edges (edges not pointingt into the bubble) it won't be removed.
     *
     * @param [in] walks bubble consisting of 2 or more paths (walks)
     * @param [in] direction direction the paths in bubble are pointing (Begin or End) needed
     * for sequence extraction
     */
    bool popBubble(const std::vector<StringGraphWalk*>& walks, const uint32_t juncture_id, const int direction);

    const std::vector<DovetailOverlap*>* overlaps_;

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
     * @brief StringGraphWalk constructir
     * @details Creates a StringGraphWalk object from the starting vertex
     *
     * @param [in] start start vertex
     */
    StringGraphWalk(const Vertex* start);

    /*!
     * @brief StringGraphWalk destructor
     */
    ~StringGraphWalk() {};

    /*!
     * @brief Getter for start vertex
     * @return start vertex
     */
    const Vertex* getStart() const {
        return start_;
    }

    /*!
     * @brief Getter for edges
     * @return vector of Edge object pointers
     */
    const std::vector<const Edge*>& getEdges() const {
        return edges_;
    }

    /*!
     * @brief Method for path extension
     * @details Adds edge to end of path
     *
     * @param [in] edge edge to be added to the path
     */
    void addEdge(const Edge* edge);

    /*!
     * @brief Method for sequence extraction
     * @details Method returns the sequence extracted from the path.
     * It equals the sequence from start vertex plus all edge labels.
     *
     * @param [out] dst sequence string
     */
    void extractSequence(std::string& dst) const;

    /*!
     * @brief Method for vertices extraction
     * @details Method appends vertices ids extracted from the path.
     *
     * @param [out] dst vertices 
     */
    void extractVertices(std::vector<const Vertex*>& dst) const;

    /*!
     * @brief Method for vertex check
     * @details Cheks whether a vertex with given identifier exists in path.
     *
     * @param [in] id vertex identifier
     * @return true if the vertex is in path
     */
    bool containsVertex(int id) const;

    /*!
     * @brief Method for edge check
     * @details Cheks whether a edge with given identifier exists in path.
     *
     * @param [in] id edge identifier
     * @return true if the edge is in path
     */
    bool containsEdge(int id) const;

private:

    const Vertex* start_;
    std::vector<const Edge*> edges_;
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
     * @brief StrinGraphNode constructor
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
    StringGraphNode(const Vertex* vertex, const Edge* edgeFromParent,
        const StringGraphNode* parent, int direction, int distance);

    /*!
     * @brief StringGraphNode desctructor
     */
    ~StringGraphNode() {};

    /*!
     * @brief Getter for vertex
     * @return vertex
     */
    const Vertex* getVertex() const {
        return vertex_;
    }

    /*!
     * @brief Getter for edge from parent
     * @return edge from parent
     */
    const Edge* getEdgeFromParent() const {
        return edgeFromParent_;
    }

    /*!
     * @brief Getter for parent vertex
     * @return parent vertex
     */
    const StringGraphNode* getParent() const {
        return parent_;
    }

    /*!
     * @brief Getter for distance
     * @return distance
     */
    int getDistance() const {
        return distance_;
    }

    /*!
     * @brief Method for path expansion
     * @details Method expands this vertex in direction and fills the input deque
     *
     * @param [in] vector of StringGraphNode object pointers
     * @return number of StringGraphNodes added to queue
     */
    size_t expand(std::vector<StringGraphNode*>& queue) const;

    /*!
     * @brief Method for walk check
     * @details Method follows the edges from parent until root node is reached. If the
     * given node is found along this path function returns true.
     *
     * @param [in] node StingGraphNode object pointer
     * @return true if node is in path to root
     */
    bool isInWalk(const StringGraphNode* node) const;

    /*!
     * @brief Method for walk search
     * @details Method follows the edges from parent until root node is reached. If the
     * given node is found along this path it is returned.
     *
     * @param [in] node StingGraphNode object pointer
     * @return node in path (if not found nullptr is returned)
     */
    const StringGraphNode* findInWalk(const StringGraphNode* node) const;

    /*!
     * @brief Method returns preceeding StringGraphNode* with given vertexId.
     * @details Method returns preceeding StringGraphNode* with given vertexId.
     *
     * @param [in] vertexId vertexId
     * @return preceeding node with given vertexId, nullptr if does not exist such node.
     */
    const StringGraphNode* rewindedTo(const int vertexId) const;

    /*!
     * @brief Method creates StringGraphWalk from this node and it's predecessors.
     * @details Method creates StringGraphWalk from this node and it's predecessors.
     *
     * @return StringGraphWalk from this node and it's predecessors.
     */
    StringGraphWalk* getWalk() const;

private:

    const Vertex* vertex_;
    const Edge* edgeFromParent_;
    const StringGraphNode* parent_;
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
     * @brief StringGraphComponent constructor
     * @details Creates a StringGraphComponent object from graph by picking out
     * vertices with given identifiers.
     *
     * @param [in] vertexIds vertex identifiers
     * @param [in] graph StringGraph object pointer
     */
    StringGraphComponent(const std::set<int> vertexIds, const StringGraph* graph);

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

    std::vector<const Vertex*>& vertices() {
      return vertices_;
    }

private:

    void extractLongestWalk();

    std::vector<const Vertex*> vertices_;
    const StringGraph* graph_;
    StringGraphWalk* walk_;
};
