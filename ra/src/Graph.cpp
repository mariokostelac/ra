
#include "Graph.hpp"
#include "UnionFind.hpp"
#include "Utils.hpp"
#include <map>
#include <set>
#include <sstream>
#include <utility>

using std::make_pair;
using std::map;
using std::pair;
using std::set;
using std::stringstream;
using std::to_string;

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
        // end of unitig
        continue;
      }

      Node* next = next_edge->dst();

      // we have to dance between graph and "mirror" graph to check if best_next(a) = b
      // and best_next(b) = a, when going in other direction
      Node* next_mirror = opposite_end_node(next);
      Edge* mirror_best_next = calculator->best_next(next_mirror);

      if (mirror_best_next != nullptr && opposite_end_node(mirror_best_next->dst()) == src) {
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
          assert(e->src()->parent_object_id() == e->dst()->parent_object_id());
          continue;
        }

        // src is tail of unitig
        e->src_ = unitig_nodes[unitig->id()].second;
      }

      if (e->dst()->parent_object_id() != -1) {
        auto unitig = unitigs_[e->dst()->parent_object_id()];
        if (unitig->head() != e->dst()) {
          assert(e->src()->parent_object_id() == e->dst()->parent_object_id());
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

  const string Graph::reads_dot() const {
    return dot(true, false);
  }

  const string Graph::unitigs_dot() const {
    return dot(false, true);
  }

  const string Graph::dot(bool include_reads, bool include_unitigs) const {
    stringstream graph_repr;

    graph_repr << "digraph {\n";

    // read/unitig begin -> read/unitig end edges
    for (auto n : nodes_) {
      if (!include_reads && n->type() == Node::Type::Read) {
        continue;
      }
      if (!include_unitigs && n->type() == Node::Type::Unitig) {
        continue;
      }
      if (n->used_end() == Node::Side::End) {
        continue;
      }

      auto src = n, dst = opposite_end_node(n);

      graph_repr << src->label() << " -> " << dst->label() << " [color=blue];\n";
    }

    for (auto e : edges_) {
      bool has_read = e->src()->type() == Node::Type::Read || e->dst()->type() == Node::Type::Read;
      bool has_unitig = e->src()->type() == Node::Type::Unitig || e->dst()->type() == Node::Type::Unitig;
      if ((has_read && !include_reads) || (has_unitig && !include_unitigs)) {
        continue;
      }

      string edge_label = this->edge_label(e);
      string label_properties = "[label=\"" + edge_label + "\", fontsize=\"9\"]";
      graph_repr << e->src()->label() << " -> " << e->dst()->label() << label_properties << ";\n";
    }
    graph_repr << "}\n";

    return graph_repr.str();
  }

  string Node::label() const {
    string type = type_ == Type::Read ? "r" : "u";
    string used_end = used_end_ == Side::Begin ? "b" : "e";

    return type + to_string(object_id()) + used_end;
  }

  const string Graph::edge_label(Edge* e) const {
    auto node_label = [this](Node* n) {
      if (n->type() == Node::Type::Read) {
        return to_string(n->object_id());
      } else {
        auto unitig = this->unitigs_[n->object_id()];
        if (n->used_end() == Node::Side::Begin) {
          return to_string(unitig->head()->object_id());
        } else {
          return to_string(unitig->tail()->object_id());
        }
      }
    };

    return node_label(e->src()) + "-" + node_label(e->dst());
  }

  Edge* BestBuddyCalculator::best_next(const Node* src) {
    auto& edges = src->edges();

    if (edges.size() != 1) {
      return nullptr;
    }

    return edges.front();
  }
};
