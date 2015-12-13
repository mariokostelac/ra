#pragma once

#include <cstdint>
#include <list>
#include <unordered_map>
#include <vector>
#include "Overlap.hpp"

using std::list;
using std::unordered_map;
using std::vector;

namespace Graph {

  class Node;
  class Edge;
  class Graph;

  class Node {

    friend class Graph;

    public:
      enum Type { Read = 0, Unitig = 1 };
      enum Side { Begin = 0, End = 1 };

      Node(Type type, uint32_t object_id, Side used_end)
        : node_id_(-1), type_(type), object_id_(object_id), used_end_(used_end) {}

      int32_t id() const {
        return node_id_;
      }

      uint32_t object_id() const {
        return object_id_;
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

    private:
      int32_t node_id_;
      Type type_;
      uint32_t object_id_;
      Side used_end_;
      list<Edge*> out_edges_;
  };

  class Edge {

    friend class Graph;

    public:
      Edge(Node* src, Node* dst) : id_(-1), src_(src), dst_(dst) {}

      int32_t id() const {
        return id_;
      }

      Node* src() const {
        return src_;
      }

      Node* dst() const {
        return dst_;
      }

    private:
      int32_t id_;
      Node* src_;
      Node* dst_;
  };

  class Graph {
    public:
    static Graph* from_overlaps(OverlapSet& overlaps);

    ~Graph();

    void add_node(Node* node);

    Node* get_or_create_node_by(Node::Type type, uint32_t object_id, Node::Side used_end);

    void add_edge(Edge* edge);

    const Node* opposite_end_node(const Node* n) const;

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

    private:
      uint64_t node_hash(Node::Type type, uint32_t object_id, Node::Side used_end) const;
      uint64_t edge_hash(Node* src, Node *dst) const;

      vector<Node*> nodes_;
      vector<Edge*> edges_;
      unordered_map<uint64_t, Node*> node_by_hash_;
      unordered_map<uint64_t, Edge*> edge_by_hash_;
  };
}
