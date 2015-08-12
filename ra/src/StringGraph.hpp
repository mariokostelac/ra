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

#include "Read.hpp"
#include "Overlap.hpp"
#include "CommonHeaders.hpp"

// trimming params
extern int READ_LEN_THRESHOLD;

// BFS params in bubble popping
extern size_t MAX_NODES;
extern int MAX_DISTANCE;
extern double MAX_DIFFERENCE;

// contig extraction params
extern size_t MAX_BRANCHES;
extern size_t MAX_START_NODES;

class Edge;
class Vertex;
class StringGraph;
class StringGraphWalk;
class StringGraphNode;
class StringGraphComponent;

/*!
 * @brief Edge class
 * @details Edge encapsulates Overlap in StringGraph.
 */
class Edge {
public:

    /*!
     * @brief Edge constructor
     * @details Creates an Edge object from a read and an overlap.
     *
     * @param [in] id edge identifier
     * @param [in] readId read identifier to determine edge direction
     * @param [in] overlap Overlap object pointer
     * @param [in] graph Graph object pointer the edge is in
     */
    Edge(int id, int readId, const Overlap* overlap, const StringGraph* graph);

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
    const Overlap* getOverlap() const {
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

    friend class Vertex;
    friend class StringGraph;

private:

    int id_;
    const Vertex* src_;
    const Vertex* dst_;
    const Overlap* overlap_;
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
     * @details Marks edge with the given identifier and its edge pair for removal.
     *
     * @param [in] id edge identifier
     */
    void markEdge(int id);

    /*!
     * @brief Method for edge marking
     * @details Marks all edges in both lists and their pairs for removal.
     */
    void markEdges();

    /*!
     * @brief Method for edge removal
     * @details Method removes marked edges in both lists but does not delete them!
     */
    void removeMarkedEdges();

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
     * Each Read becomes a Vertex and from each Overlap two Edges are created,
     * one from A to B and one from B to A (A and B are reads in Overlap).
     *
     * @param [in] reads vector of Read object pointers
     * @param [in] overlaps vector of Overlap object pointers
     */
    StringGraph(const std::vector<Read*>& reads, const std::vector<Overlap*>& overlaps);

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
        return vertices_[verticesDict_.at(id)];
    }

    /*!
     * @brief Getter for number of vertices in graph
     * @return number of vertices
     */
    size_t getNumVertices() const {
        return vertices_.size();
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
     */
    void popBubbles();

    /*!
     * @brief Method for graph simplification
     * @details Method calls both trimming and bubble popping in an alternating
     * fashion until no changes occur in graph.
     */
    void simplify();

    /*!
     * @brief Method for overlap extracion
     * @details Method returns all overlaps present in graph.
     *
     * @param [out] dst vector of Overlap object pointers
     * @param [in] view if true Overlap objects are not cloned to dst
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

private:

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
     */
    void findBubbleWalks(std::vector<StringGraphWalk*>& dst, const Vertex* root, int direction);

    /*!
     * @brief Method for bubble juncture check
     * @details Method checks if openedQue_ or closedQue_ contain duplicates (i.e. junctures).
     *
     * @param [in] rootNode root StringGraphNode object pointer
     * @return juncture StingGraphNode object pointer if found
     */
    const StringGraphNode* bubbleJuncture(StringGraphNode* rootNode) const;

    /*!
     * @brief Method for bubble extraction
     * @details Method finds all StingGraphNodes which encapsule the same vertex as the juncture
     * and backtracts the path from each of them to the root. For each path a StringGraphWalk
     * object is created from root to juncture.
     *
     * @param [out] dst vector of found paths
     * @param [in] root Vertex object pointer
     * @param [in] junctureNode juncture StringGraphNode object pointer
     */
    void extractBubbleWalks(std::vector<StringGraphWalk*>& dst, const Vertex* root,
        const StringGraphNode* junctureNode) const;

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
    bool popBubble(const std::vector<StringGraphWalk*>& walks, int direction);

    /*!
     * @brief Method for path checking
     * @details Method checks if the given path has any external edges
     *
     * @param [in] id index of the path in bubble
     * @param [in] walks bubble consisting of 2 or more paths (walks)
     */
    bool isValidWalk(int id, const std::vector<StringGraphWalk*>& walks) const;

    /*!
     * @brief Method for deletion of vertices and edges
     * @details Method first removes edges from lists of each vertex in marked_.
     * Afterwards it deletes all marked edges and vertices from the graph.
     */
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
     * @param [in] queue deque of StringGraphNode object pointers
     * @return number of StringGraphNodes added to queue
     */
    size_t expand(std::deque<StringGraphNode*>& queue);

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
    friend class ContigExtractor;

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

private:

    void extractLongestWalk();

    std::vector<const Vertex*> vertices_;
    const StringGraph* graph_;
    StringGraphWalk* walk_;
};

