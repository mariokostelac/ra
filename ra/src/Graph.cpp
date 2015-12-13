
#include "Graph.hpp"
#include "Utils.hpp"

namespace Graph {

  Graph* Graph::from_overlaps(OverlapSet& overlaps) {
    Graph* g = new Graph;

    for (uint32_t i = 0; i < overlaps.size(); ++i) {
      auto overlap = overlaps[i];
      debug("Graph::from_overlaps - processing overlap %d %d\n", overlap->a(), overlap->b());
      auto a = overlap->a(), b = overlap->b();

      if (overlap->is_using_suffix(a) && overlap->is_using_prefix(b)) {
        {
          // ---->
          //   --||>
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst));
        }
        {
          // ||-->
          //   ---->
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst));
        }
      } else if (overlap->is_using_prefix(a) && overlap->is_using_suffix(b)) {
        {
          //   ---->
          // ||-->
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst));
        }
        {
          //   --||>
          // ---->
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst));
        }
      } else if (overlap->is_using_suffix(a) && overlap->is_using_suffix(b)) {
        {
          // ---->
          //   <--||
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst));
        }
        {
          // ||-->
          //   <----
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst));
        }
      } else if (overlap->is_using_prefix(a) && overlap->is_using_prefix(b)) {
        {
          //   ---|>
          // <----
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst));
        }
        {
          //   ---->
          // <|---
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst));
        }
      } else {
        assert(false);
      }
    }

    return g;
  }

  Node* Graph::get_or_create_node_by(Node::Type type, uint32_t object_id, Node::Side used_end) {
    debug("Graph::get_or_create_node_by %d %d %d\n", type, object_id, used_end);
    uint64_t hash = node_hash(type, object_id, used_end);
    debug("Graph::get_or_create_node_by hash %#018llX\n", hash);
    if (node_by_hash_.count(hash) == 0) {
      add_node(new Node(type, object_id, used_end));
    }

    return node_by_hash_[hash];
  }


  void Graph::add_node(Node* node) {
    assert(node->id() == -1);

    node->node_id_ = nodes_.size();
    nodes_.push_back(node);

    uint64_t hash = node_hash(node->type(), node->object_id(), node->used_end());
    node_by_hash_[hash] = node;
    debug("Graph::add_node hash %#018llX\n", hash);
  }

  uint64_t Graph::node_hash(Node::Type type, uint32_t object_id, Node::Side used_end) {
    uint64_t hash = 0LL;
    hash = hash ^ (uint16_t) type;
    hash = hash << 16;
    hash = hash ^ (uint16_t) used_end;
    hash = hash << 32;
    hash = hash ^ object_id;
    return hash;
  }

  void Graph::add_edge(Edge* edge) {
    assert(edge->id() == -1);

    debug("Graph::add_edge %#018llX\n", edge);

    edge->id_ = edges_.size();
    edges_.push_back(edge);
    edge->src()->out_edges_.push_back(edge);

    uint64_t hash = edge_hash(edge->src(), edge->dst());
    debug("Graph::add_edge hash %#018llX\n", hash);
    edge_by_hash_[hash] = edge;
  }

  uint64_t Graph::edge_hash(Node* src, Node* dst) {
    debug("Graph::edge_hash %#018llX %#018llX\n", src, dst);
    uint64_t hash = 0LL;
    hash = hash ^ (uint32_t) src->id();
    hash = hash << 32;
    hash = hash ^ (uint32_t) dst->id();
    return hash;
  }
};
