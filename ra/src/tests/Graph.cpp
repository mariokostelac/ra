#include <string>
#include "gtest/gtest.h"
#include "ra.hpp"

using namespace std;

const int THREAD_LEN = 1;

TEST(Graph, get_or_create_node_by_assigns_correct_values) {
  int object_id = 0x12345678;
  auto used_end = Graph::Node::Side::Begin;
  auto object_type = Graph::Node::Type::Read;

  Graph::Graph g;
  auto node = g.get_or_create_node_by(object_type, object_id, used_end);

  ASSERT_TRUE(node != nullptr);
  ASSERT_EQ(node->id(), 0);
  ASSERT_EQ(node->type(), object_type);
  ASSERT_EQ(node->object_id(), object_id);
  ASSERT_EQ(node->used_end(), used_end);
}

TEST(Graph, get_or_create_node_by_inserts_node_in_nodes_collection) {
  int object_id = 0x12345678;
  auto used_end = Graph::Node::Side::Begin;
  auto object_type = Graph::Node::Type::Read;

  Graph::Graph g;

  ASSERT_EQ(0, g.nodes_count());
  g.get_or_create_node_by(object_type, object_id, used_end);
  ASSERT_EQ(1, g.nodes_count());
}

TEST(Graph, get_or_create_node_by_returns_same_node_on_second_call) {
  int object_id = 0x0A0;
  auto used_end = Graph::Node::Side::End;
  auto object_type = Graph::Node::Type::Unitig;

  Graph::Graph g;
  auto node = g.get_or_create_node_by(object_type, object_id, used_end);
  auto node1 = g.get_or_create_node_by(object_type, object_id, used_end);

  ASSERT_EQ(node, node1);
  ASSERT_EQ(1, g.nodes_count());
}

TEST(Graph, get_or_create_node_by_returns_new_node_on_second_call) {
  int object_id1 = 0x0A0, object_id2 = 0x0B0;
  auto used_end1 = Graph::Node::Side::Begin, used_end2 = Graph::Node::Side::End;
  auto object_type1 = Graph::Node::Type::Unitig, object_type2 = Graph::Node::Type::Read;

  Graph::Graph g;
  auto node = g.get_or_create_node_by(object_type1, object_id1, used_end1);
  auto node1 = g.get_or_create_node_by(object_type2, object_id2, used_end2);

  ASSERT_EQ(node->id(), 0);
  ASSERT_EQ(node->type(), object_type1);
  ASSERT_EQ(node->object_id(), object_id1);
  ASSERT_EQ(node->used_end(), used_end1);

  ASSERT_EQ(node1->id(), 1);
  ASSERT_EQ(node1->type(), object_type2);
  ASSERT_EQ(node1->object_id(), object_id2);
  ASSERT_EQ(node1->used_end(), used_end2);
}

TEST(Graph, add_edge_adds_edge_to_graph) {
  int object_id1 = 0x0A0, object_id2 = 0x0B0;
  auto used_end = Graph::Node::Side::End;
  auto object_type = Graph::Node::Type::Unitig;

  Graph::Graph g;
  auto node_a = g.get_or_create_node_by(object_type, object_id1, used_end);
  auto node_b = g.get_or_create_node_by(object_type, object_id2, used_end);

  auto edge = new Graph::Edge(node_a, node_b);

  ASSERT_EQ(0, g.edges_count());
  g.add_edge(edge);
  ASSERT_EQ(1, g.edges_count());
}


