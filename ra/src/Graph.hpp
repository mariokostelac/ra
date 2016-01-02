#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include "Overlap.hpp"

using std::list;
using std::map;
using std::string;
using std::unordered_map;
using std::vector;

namespace Graph {

  class Node;
  class Edge;
  class Graph;
  class GraphWalk;
  class BestBuddyCalculator;
  class Unitig;

  class Node {

    friend class Graph;

    public:
      enum Type { Read = 0, Unitig = 1 };
      enum Side { Begin = 0, End = 1 };

      Node(Type type, uint32_t object_id, Side used_end)
        : node_id_(-1), type_(type), object_id_(object_id), parent_object_id_(-1), used_end_(used_end) {}

      int32_t id() const {
        return node_id_;
      }

      uint32_t object_id() const {
        return object_id_;
      }

      int32_t parent_object_id() const {
        return parent_object_id_;
      }

      Type type() const {
        return type_;
      }

      Side used_end() const {
        return used_end_;
      }

      const list<Edge*> edges() const {
        return out_edges_;
      }

      bool is_read() const {
        return type() == Type::Read;
      }

      bool is_unitig() const {
        return type() == Type::Unitig;
      }

      string label() const;

    private:
      int32_t node_id_;
      Type type_;
      uint32_t object_id_;
      int32_t parent_object_id_;
      Side used_end_;
      list<Edge*> out_edges_;
  };

  class Edge {

    friend class Graph;

    public:
      Edge(Node* src, Node* dst, const Overlap* overlap) : id_(-1), src_(src), dst_(dst), overlap_(overlap) {}

      int32_t id() const {
        return id_;
      }

      Node* src() const {
        return src_;
      }

      Node* dst() const {
        return dst_;
      }

      const Overlap* overlap() const {
        return overlap_;
      }

      const string label() const;

    private:
      int32_t id_;
      Node* src_;
      Node* dst_;
      const Overlap* overlap_;
  };

  class Graph {
    public:
    static Graph* from_overlaps(OverlapSet& overlaps);

    ~Graph();

    void add_node(Node* node);

    Node* get_or_create_node_by(Node::Type type, uint32_t object_id, Node::Side used_end);

    void add_edge(Edge* edge);

    void add_unitig(Unitig* unitig);

    Node* opposite_end_node(const Node* n) const;

    void convert_to_unitig_graph(BestBuddyCalculator* calculator);

    uint32_t nodes_count() const {
      return nodes_.size();
    }

    uint32_t edges_count() const {
      return edges_.size();
    }

    const vector<Node*> nodes() const {
      return nodes_;
    }

    const vector<Edge*> edges() const {
      return edges_;
    }

    const vector<Unitig*> unitigs() const {
      return unitigs_;
    }

    const string reads_dot() const;
    const string unitigs_dot() const;
    const string extract_sequence(Unitig* u) const;

    private:
      const string dot(bool include_reads, bool include_unitigs) const;
      const string edge_label(Edge* e) const;
      uint64_t node_hash(Node::Type type, uint32_t object_id, Node::Side used_end) const;
      uint64_t edge_hash(Node* src, Node *dst) const;
      void rewire_graph_with_unitigs();

      vector<Node*> nodes_;
      vector<Edge*> edges_;
      vector<Unitig*> unitigs_;
      map<uint32_t, const Read*> reads_;
      unordered_map<uint64_t, Node*> node_by_hash_;
      unordered_map<uint64_t, Edge*> edge_by_hash_;
  };

  class BestBuddyCalculator {
    public:
      BestBuddyCalculator(Graph* graph) : graph_(graph) {}
      virtual ~BestBuddyCalculator() {}

      virtual Edge* best_next(const Node* src);

    protected:
      Graph* graph_;
  };

  class GraphWalk {
    public:
      GraphWalk(Node* head) : head_(head), tail_(head) {}

      Node* head() const {
        return head_;
      }

      Node* tail() const {
        return tail_;
      }

      void add_next_edge(Edge* edge) {
        edges_.push_back(edge);
        tail_ = edge->dst();
      }

      const vector<Edge*> edges() {
        return edges_;
      }

    protected:
      Node* head_;
      Node* tail_;
      vector<Edge*> edges_;
  };

  class Unitig {
    friend class Graph;

    public:
      Unitig(GraphWalk* walk) : id_(-1), walk_(walk) {}

      ~Unitig() { delete walk_; }

      int32_t id() const {
        return id_;
      }

      Node* head() const {
        return walk_->head();
      }

      Node* tail() const {
        return walk_->tail();
      }

      const vector<Edge*> edges() {
        return walk_->edges();
      }

    private:
      int32_t id_;
      GraphWalk* walk_;
  };
}
