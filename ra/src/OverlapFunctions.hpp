
#pragma once

#include "Read.hpp"
#include "DovetailOverlap.hpp"
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
void filterTransitiveOverlaps(std::vector<DovetailOverlap*>& dst, const std::vector<DovetailOverlap*>& overlaps,
    int threadLen, bool view = true);

/*!
 * @brief Method for overlaping reads
 * @details Method creates EnhancesSuffixArray objects from reads and uses them for pattern
 * matching, i.e. prefix-sufix overlaps. It also creates reverse complements of reads needed
 * to get all types of overlaps.
 *
 * @param [out] dst vector of Overlap objects pointers
 * @param [in] reads vector of Read objects pointers
 * @param [in] minOverlapLen minimal length of overlaps considered
 * @param [in] threadLen number of threads
 * @param [in] path path to file where the EnhancedSuffixArray objects are cached to speed up
 * future runs on the same data
 */
void overlapReads(std::vector<DovetailOverlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path);

std::pair<int, int> calc_forced_hangs(uint32_t a_lo, uint32_t a_hi, uint32_t a_len, bool a_rc,
    uint32_t b_lo, uint32_t b_hi, uint32_t b_len, bool b_rc);

DovetailOverlap* forced_dovetail_overlap(const Overlap* overlap, bool calc_error_rates);
