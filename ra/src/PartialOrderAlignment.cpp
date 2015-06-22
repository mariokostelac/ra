/*!
 * @file PartialOrderAlignment.cpp
 *
 * @brief Cpppoa wrapper source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Jun 11, 2015
 */


#include "PartialOrderAlignment.hpp"

#include "../vendor/cpppoa/poa.hpp"

extern std::string consensus(const Contig* contig, const std::vector<Read*>& reads) {

    std::vector<std::string> sequences;

    for (const auto& part : contig->getParts()) {

        int id = std::get<0>(part);
        int type = std::get<1>(part);

        sequences.emplace_back(type ? reads[id]->getReverseComplement() : reads[id]->getSequence());
    }

    return poa_consensus(sequences);
}
