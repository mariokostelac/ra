#include "gtest/gtest.h"
#include "ra.hpp"

using namespace std;

const int THREAD_LEN = 1;

int32_t chain_len(Graph::Node* curr_node) {
  int32_t len = 0;
  while (true) {
    len++;
    if (curr_node->edges().size() != 1) {
      break;
    }
    curr_node = curr_node->edges().front()->dst();
  }

  return len;
}

vector<Read*> reads_from_seqs(vector<string>& seqs) {
  vector<Read*> reads;

  for (uint32_t i = 0; i < seqs.size(); ++i) {
    reads.push_back(new Read(i, std::to_string(i), seqs[i], "", 1));
  }

  return reads;
}

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

  auto edge = new Graph::Edge(node_a, node_b, nullptr);

  ASSERT_EQ(0, g.edges_count());
  g.add_edge(edge);
  ASSERT_EQ(1, g.edges_count());
}


TEST(Graph, from_overlaps_assembles_chain_to_two_chains) {
  string genome = "AACCGGTATC";

  vector<string> seqs;
  seqs.push_back(genome.substr(0, 4));
  seqs.push_back(genome.substr(2, 4));
  seqs.push_back(genome.substr(4, 4));
  seqs.push_back(genome.substr(6, 4));

  auto reads = reads_from_seqs(seqs);
  vector<Overlap*> overlaps;

  overlapReads(overlaps, reads, 2);

  ASSERT_EQ(3, overlaps.size());

  Graph::Graph *g = Graph::Graph::from_overlaps(overlaps);

  ASSERT_EQ(4 * 2, g->nodes_count()); // every seq x 2 ends
  ASSERT_EQ(3 * 2, g->edges_count()); // every overlap x 2 directions

  unordered_map<int, int> chain_lengths;
  int max_chain_len = 0;
  for (auto node : g->nodes()) {
    ASSERT_TRUE(node->edges().size() <= 1);

    auto curr_chain_len = chain_len(node);
    if (!chain_lengths.count(curr_chain_len)) {
      chain_lengths[curr_chain_len] = 1;
    } else {
      chain_lengths[curr_chain_len]++;
    }
    max_chain_len = max(max_chain_len, curr_chain_len);
  }

  ASSERT_EQ(2, chain_lengths[1]); // two end nodes
  ASSERT_EQ(2, chain_lengths[4]); // two start nodes
  ASSERT_EQ(4, max_chain_len);

  delete g;
}

TEST(Graph, from_overlaps_assembles_chain_to_two_chains2) {
  string genome = "AACTGCCCAT";

  vector<string> seqs;
  seqs.push_back(reverseComplement(genome.substr(4, 4)));
  seqs.push_back(genome.substr(2, 4));
  seqs.push_back(genome.substr(8, 4));
  seqs.push_back(genome.substr(6, 4));
  seqs.push_back(genome.substr(0, 4));

  auto reads = reads_from_seqs(seqs);
  vector<Overlap*> overlaps;

  overlapReads(overlaps, reads, 2);

  ASSERT_EQ(4, overlaps.size());

  Graph::Graph *g = Graph::Graph::from_overlaps(overlaps);

  ASSERT_EQ(5 * 2, g->nodes_count()); // every seq x 2 ends
  ASSERT_EQ(4 * 2, g->edges_count()); // every overlap x 2 directions

  unordered_map<int, int> chain_lengths;
  int max_chain_len = 0;
  for (auto node : g->nodes()) {
    ASSERT_TRUE(node->edges().size() <= 1);

    auto curr_chain_len = chain_len(node);
    if (!chain_lengths.count(curr_chain_len)) {
      chain_lengths[curr_chain_len] = 1;
    } else {
      chain_lengths[curr_chain_len]++;
    }
    max_chain_len = max(max_chain_len, curr_chain_len);
  }

  ASSERT_EQ(2, chain_lengths[1]); // two end nodes
  ASSERT_EQ(2, chain_lengths[5]); // two start nodes
  ASSERT_EQ(5, max_chain_len);

  delete g;
}

TEST(Graph, every_node_has_its_pair) {
  string genome = "AACTGCCCAT";

  vector<string> seqs;
  seqs.push_back(reverseComplement(genome.substr(4, 4)));
  seqs.push_back(genome.substr(2, 4));
  seqs.push_back(genome.substr(8, 4));
  seqs.push_back(genome.substr(6, 4));
  seqs.push_back(genome.substr(0, 4));

  auto reads = reads_from_seqs(seqs);
  vector<Overlap*> overlaps;

  overlapReads(overlaps, reads, 2);

  ASSERT_EQ(4, overlaps.size());

  Graph::Graph *g = Graph::Graph::from_overlaps(overlaps);

  for (auto n : g->nodes()) {
    const Graph::Node* pair = g->opposite_end_node(n);
    ASSERT_TRUE(nullptr != pair);
    ASSERT_EQ(n->object_id(), pair->object_id());
    ASSERT_EQ(n->type(), pair->type());
    ASSERT_TRUE(n->used_end() != pair->used_end());
  }

  delete g;
}

TEST(Graph, convert_to_unitig_graph_creates_4_unitig_nodes) {
  string genome = "AACCGGTATC";

  vector<string> seqs;
  seqs.push_back(genome.substr(0, 4));
  seqs.push_back(genome.substr(2, 4));
  seqs.push_back(genome.substr(4, 4));
  seqs.push_back(genome.substr(6, 4));

  auto reads = reads_from_seqs(seqs);
  vector<Overlap*> overlaps;

  overlapReads(overlaps, reads, 2);

  ASSERT_EQ(3, overlaps.size());

  Graph::Graph *g = Graph::Graph::from_overlaps(overlaps);
  Graph::BestBuddyCalculator* calculator = new Graph::BestBuddyCalculator(g);
  g->convert_to_unitig_graph(calculator);

  uint32_t unitig_nodes = 0;
  for (auto n : g->nodes()) {
    unitig_nodes += n->type() == Graph::Node::Unitig;
  }

  ASSERT_EQ(4, unitig_nodes);

  delete g;
  delete calculator;
}
