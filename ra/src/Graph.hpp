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
      enum Type { Read, Unitig };
      enum Side { Begin, End };

      Node(Type type, uint32_t object_id, Side used_end) {
        node_id_ = -1;
        type_ = type;
        object_id_ = object_id;
        used_end_ = used_end_;
      }

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

    private:
      int32_t node_id_;
      uint32_t object_id_;
      Type type_;
      Side used_end_;
      list<Edge*> out_edges_;
  };

  class Edge {

    friend class Graph;

    public:
      Edge(Node* src, Node* dst) {
        id_ = -1;
        src_ = src;
        dst_ = dst;
      }

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

    void add_node(Node* node);
    Node* get_or_create_node_by(Node::Type type, uint32_t object_id, Node::Side used_end);

    void add_edge(Edge* edge);

    private:
      uint64_t node_hash(Node::Type type, uint32_t object_id, Node::Side used_end);
      uint64_t edge_hash(Node* src, Node *dst);

      vector<Node*> nodes_;
      vector<Edge*> edges_;
      unordered_map<uint64_t, Node*> node_by_hash_;
      unordered_map<uint64_t, Edge*> edge_by_hash_;
  };
}
