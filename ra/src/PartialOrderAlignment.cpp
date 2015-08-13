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

const double THRESHOLD = 0.05;

extern std::string consensus(const Contig* contig, const std::vector<Read*>& reads) {

    int size = contig->getParts().size();
    if (size == 0) {
      return "";
    }

    auto& first = contig->getParts().front();
    string result = first.type() ? reads[first.src]->getReverseComplement() : reads[first.src]->getSequence();

    for (int i = 1; i < size; ++i) {
      const auto& curr = contig->getParts()[i];
      const auto& curr_seq = curr.type() ? reads[curr.src]->getReverseComplement() : reads[curr.src]->getSequence();
      const int break_index = std::max(0, (int) (curr.offset - curr_seq.length()*THRESHOLD));

      const auto& fixed = result.substr(0, break_index);
      vector<string> seqs;
      seqs.emplace_back(result.substr(break_index, result.size() - break_index));
      seqs.emplace_back(curr_seq);

      const auto& aligned = poa_consensus(seqs);

      result = fixed + aligned;

      fprintf(stderr, "%d o:%d ovllen:%lu reslen:%lu\n", i, break_index, aligned.length(), result.length());
    }

    return result;
}
