
#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief Method for containment overlaps filtering
 * @details Method picks overlaps in which both reads are not contained in some other reads.
 *
 * @param [out] dst vector of non containment Overlap object pointers
 * @param [in] overlaps vector of Overlap object pointers
 * @param [in] reads vector of Read object pointers (their coverage is updated if they contain other reads)
 * @param [in] view if true Overlap objects are not cloned to dst
 */
void filterContainedOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps,
    std::vector<Read*>& reads, bool view = true);

/*!
 * @brief Method for transitive overlaps filtering
 * @details Method picks overlaps which are not transitive considerin any other two overlaps.
 * It is done in parallel.
 *
 * @param [out] dst vector of non transitive Overlap object pointers
 * @param [in] overlaps vector of Overlap object pointers
 * @param [in] threadLen number of threads
 * @param [in] view if true Overlap objects are not cloned to dst
 */
void filterTransitiveOverlaps(std::vector<Overlap*>& dst, const std::vector<Overlap*>& overlaps,
    int threadLen, bool view = true);
