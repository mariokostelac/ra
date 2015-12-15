
#include "Graph.hpp"
#include "UnionFind.hpp"
#include "Utils.hpp"
#include <map>
#include <set>
#include <utility>

using std::make_pair;
using std::map;
using std::pair;
using std::set;

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
          g->add_edge(new Edge(src, dst, overlap));
        }
        {
          // ||-->
          //   ---->
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst, overlap));
        }
      } else if (overlap->is_using_prefix(a) && overlap->is_using_suffix(b)) {
        {
          //   ---->
          // ||-->
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst, overlap));
        }
        {
          //   --||>
          // ---->
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst, overlap));
        }
      } else if (overlap->is_using_suffix(a) && overlap->is_using_suffix(b)) {
        {
          // ---->
          //   <--||
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst, overlap));
        }
        {
          // ||-->
          //   <----
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst, overlap));
        }
      } else if (overlap->is_using_prefix(a) && overlap->is_using_prefix(b)) {
        {
          //   ---|>
          // <----
          Node* src = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::End);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst, overlap));
        }
        {
          //   ---->
          // <|---
          Node* src = g->get_or_create_node_by(Node::Type::Read, a, Node::Side::Begin);
          Node* dst = g->get_or_create_node_by(Node::Type::Read, b, Node::Side::End);
          assert(src != nullptr);
          assert(dst != nullptr);
          g->add_edge(new Edge(src, dst, overlap));
        }
      } else {
        assert(false);
      }
    }

    return g;
  }

  Graph::~Graph() {
    for (auto node : nodes_) {
      delete node;
    }
    for (auto edge : edges_) {
      delete edge;
    }
    for (auto unitig : unitigs_) {
      delete unitig;
    }
    debug("Graph destroyed\n");
  }

  void Graph::convert_to_unitig_graph(BestBuddyCalculator* calculator) {
    UnionFind unitig_id(nodes_.size());
    vector<Edge*> next_in_unitig(nodes_.size());

    // map node -> unitig_id
    for (uint32_t i = 0; i < nodes_.size(); ++i) {
      auto src = nodes_[i];

      Edge* next_edge = calculator->best_next(src);
      if (next_edge == nullptr) {
        // end of graph
        continue;
      }

      Node* next = next_edge->dst();

      // we have to dance between graph and "mirror" graph to check if best_next(a) = b
      // and best_next(b) = a, when going in other direction
      Node* next_mirror = opposite_end_node(next);

      if (opposite_end_node(calculator->best_next(next_mirror)->dst()) == src) {
        debug("Graph::unitig_graph join %u %u\n", src->id(), next->id());
        unitig_id.join(src->id(), next->id());
        next_in_unitig[src->id()] = next_edge;
      }
    }

    // find unitig starts by "collecting" nodes. Every time some node is able to collect
    // uncollected nodes in same unitig, it is promoted to the the unitig head.
    map<uint32_t, uint32_t> unitig_start;
    set<Node*> collected;

    for (uint32_t i = 0; i < edges_.size(); ++i) {
      auto e = edges_[i];
      Node* curr = e->src();

      if (collected.count(curr)) {
        continue;
      }

      if (next_in_unitig[curr->id()] == nullptr) {
        // last in unitig or unitig consisting of one read.
        continue;
      }

      unitig_start[unitig_id.find(curr->id())] = curr->id();

      // collect
      collected.insert(curr);
      while (next_in_unitig[curr->id()] != nullptr) {
        curr = next_in_unitig[curr->id()]->dst();
        collected.insert(curr);
      }
    }

    for (auto kv : unitig_start) {
      auto start_node = nodes_[kv.second];
      GraphWalk* walk =  new GraphWalk(start_node);

      auto curr_node = start_node;
      while (next_in_unitig[curr_node->id()] != nullptr) {
        auto edge = next_in_unitig[curr_node->id()];
        walk->add_next_edge(edge);
        curr_node = edge->dst();
      }

      add_unitig(new Unitig(walk));
    }

    rewire_graph_with_unitigs();
  }

  void Graph::rewire_graph_with_unitigs() {

    // get start and end nodes of unitigs
    vector<pair<Node*, Node*>> unitig_nodes(unitigs_.size());
    for (auto n : nodes_) {
      if (n->type() != Node::Type::Unitig) {
        continue;
      }

      if (n->used_end() == Node::Side::Begin) {
        unitig_nodes[n->object_id()].first = n;
      } else {
        unitig_nodes[n->object_id()].second = n;
      }
    }

    // rewire edges coming into heads and going out from tails
    for (auto e : edges_) {
      if (e->src()->parent_object_id() != -1) {
        auto unitig = unitigs_[e->src()->parent_object_id()];
        if (unitig->tail() != e->src()) {
          continue;
        }

        // src is tail of unitig
        e->src_ = unitig_nodes[unitig->id()].second;
      }

      if (e->dst()->parent_object_id() != -1) {
        auto unitig = unitigs_[e->dst()->parent_object_id()];
        if (unitig->head() !=  e->dst()) {
          continue;
        }

        // dst is start of unitig
        e->dst_ = unitig_nodes[unitig->id()].first;
      }
    }
  }

  Node* Graph::opposite_end_node(const Node* n) const {
    Node::Side opposite_end = n->used_end() == Node::Side::Begin ? Node::Side::End : Node::Side::Begin;
    uint64_t hash = node_hash(n->type(), n->object_id(), opposite_end);
    return node_by_hash_.at(hash);
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

  uint64_t Graph::node_hash(Node::Type type, uint32_t object_id, Node::Side used_end) const {
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

  uint64_t Graph::edge_hash(Node* src, Node* dst) const {
    debug("Graph::edge_hash %#018llX %#018llX\n", src, dst);
    uint64_t hash = 0LL;
    hash = hash ^ (uint32_t) src->id();
    hash = hash << 32;
    hash = hash ^ (uint32_t) dst->id();
    return hash;
  }

  void Graph::add_unitig(Unitig* unitig) {
    assert(unitig->id() == -1);

    unitig->id_ = unitigs_.size();
    unitigs_.push_back(unitig);

    get_or_create_node_by(Node::Type::Unitig, unitig->id(), Node::Side::Begin);
    get_or_create_node_by(Node::Type::Unitig, unitig->id(), Node::Side::End);

    auto curr_node = unitig->head();
    curr_node->parent_object_id_ = unitig->id();
    for (auto e : unitig->edges()) {
      e->dst()->parent_object_id_ = unitig->id();
    }
  }

  Edge* BestBuddyCalculator::best_next(const Node* src) {
    auto& edges = src->edges();

    if (edges.size() == 0) {
      return nullptr;
    }

    Edge* best_edge = edges.front();
    uint32_t best_length = best_edge->overlap()->length(src->object_id());

    for (auto& edge : edges) {
      uint32_t curr_length = edge->overlap()->length(src->object_id());
      if (curr_length >= best_length) {
        best_edge = edge;
        best_length = curr_length;
      }
    }

    return best_edge;
  }
};
