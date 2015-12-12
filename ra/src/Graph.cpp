
#include "Graph.hpp"

namespace Graph {

  Graph* Graph::from_overlaps(OverlapSet& overlaps) {
    Graph* g = new Graph;

    for (uint32_t i = 0; i < overlaps.size(); ++i) {
      auto overlap = overlaps[i];
      auto a = overlap->a(), b = overlap->b();

      if (overlap->is_using_suffix(a) && overlap->is_using_prefix(b)) {
        {
          // ---->
          //   --||>
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          g->add_edge(new Edge(src, dst));
        }
        {
          // ||-->
          //   ---->
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          g->add_edge(new Edge(src, dst));
        }
      } else if (overlap->is_using_prefix(a) && overlap->is_using_suffix(b)) {
        {
          //   ---->
          // ||-->
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          g->add_edge(new Edge(src, dst));
        }
        {
          //   --||>
          // ---->
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          g->add_edge(new Edge(src, dst));
        }
      } else if (overlap->is_using_suffix(a) && overlap->is_using_suffix(b)) {
        {
          // ---->
          //   <--||
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          g->add_edge(new Edge(src, dst));
        }
        {
          // ||-->
          //   <----
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          g->add_edge(new Edge(src, dst));
        }
      } else if (overlap->is_using_prefix(a) && overlap->is_using_prefix(b)) {
        {
          //   ---|>
          // <----
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          g->add_edge(new Edge(src, dst));
        }
        {
          //   ---->
          // <|---
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          g->add_edge(new Edge(src, dst));
        }
      } else {
        assert(false);
      }
    }

    return g;
  }

  Node* Graph::get_or_create_node_by(Node::Type type, uint32_t object_id, Node::Side used_end) {
    auto hash = node_hash(type, object_id, used_end);
    if (node_by_hash_.count(hash) == 0) {
      add_node(new Node(type, object_id, used_end));
    }

    return node_by_hash_[hash];
  }

  void Graph::add_node(Node* node) {
    assert(node->id() == -1);

    node->node_id_ = nodes_.size();
    nodes_.push_back(node);
    node_by_hash_[node_hash(node->type(), node->object_id(), node->used_end())] = node;
  }

  uint64_t Graph::node_hash(Node::Type type, uint32_t object_id, Node::Side used_end) {
    uint64_t hash = 0;
    hash = hash | (uint32_t) type;
    hash = hash << 16;
    hash = hash | (uint32_t) used_end;
    hash = hash << 16;
    hash = hash | object_id;
    return hash;
  }

  void Graph::add_edge(Edge* edge) {
    assert(edge->id() == -1);

    edge->id_ = edges_.size();
    edges_.push_back(edge);
    edge_by_hash_[edge_hash(edge->src(), edge->dst())] = edge;
  }

  uint64_t Graph::edge_hash(Node* src, Node* dst) {
    uint64_t hash = 0;
    hash = hash | (uint32_t) src->id();
    hash = hash << 32;
    hash = hash | (uint32_t) dst->id();
    return hash;
  }
};
