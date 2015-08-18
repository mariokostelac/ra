/*!
 * @file PartialOrderAlignment.cpp
 *
 * @brief Cpppoa wrapper source file
 *
 * @author mariokostelac (mariokostelac@gmail.com)
 * @date Jun 11, 2015
 */


#include "PartialOrderAlignment.hpp"

#include "../vendor/cpppoa/poa.hpp"
#include "../vendor/cpppoa/graph.hpp"
#include "../vendor/cpppoa/alignment.hpp"

const double THRESHOLD = 0.05;
const double BAND_PERCENTAGE = 0.1;

extern std::string consensus(const Contig* contig, const std::vector<Read*>& reads) {

    int size = contig->getParts().size();
    if (size == 0) {
      return "";
    }

    auto& first = contig->getParts().front();
    POA::Graph graph(first.type() ? reads[first.src]->getReverseComplement() : reads[first.src]->getSequence(), "seq0");

    for (int i = 1; i < size; ++i) {
      const auto& curr = contig->getParts()[i];
      const auto& curr_seq = curr.type() ? reads[curr.src]->getReverseComplement() : reads[curr.src]->getSequence();
      const int offset = std::max((int) (curr.offset - THRESHOLD * curr_seq.length()), (int) (BAND_PERCENTAGE * curr_seq.length()));

      Timer t;
      t.start();
      POA::Alignment aln(const_cast<string&>(curr_seq), graph);
      aln.align_banded_starting_at(offset, BAND_PERCENTAGE * curr_seq.length());
      t.stop();
      t.print("consensus", "poa");

      graph.insertSequenceAlignment(aln, curr_seq, "seq" + std::to_string(i));
    }

    string consensus;
    graph.generate_consensus(&consensus);
    return consensus;
}
