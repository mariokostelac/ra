/*!
 * @file Preprocess.hpp
 *
 * @brief Preprocess header file
 * @details Preprocess includes error correction and filtering of reads.\n
 * Error correction was rewritten and modified from SGA (String Graph Assembler): \n
 *     Paper: Efficient de novo assembly of large genomes using compressed data structures (+ supplemmental material) \n
 *     Authors: Jared T. Simpson and Richard Durbin \n
 *     Github: https://github.com/jts/sga \n
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Apr 27, 2015
 */

#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief KmerDistribution class
 * @details It holds a histogram of k-mer frequencies and is needed for error correction.
 */
class KmerDistribution {
public:

    /*!
     * @brief KmerDistribution constructor
     */
    KmerDistribution() {}

    /*!
     * @brief KmerDistribution destructor
     */
    ~KmerDistribution() {}

    /*!
     * @brief Method for histogram filling
     */
    void add(int kmerCount) {
        histogram_[kmerCount]++;
    }

    /*!
     * @brief Method for cumulative proportion
     * @details Method returns the proportion of the distribution whith frequencies less
     * than or equal to n.
     *
     * @param [in] n kmer frequency
     * @return cumulative proportion
     */
    double cumulativeProportion(int n) const;

    /*!
     * @brief Method for error boundary
     * @details Method returns the boundary of the left tail which holds erroneous kmers.
     * It looks for first 2 frequencies that satisfy the following:
     * histogram[i] / histogram[i + 1] < ratio.
     *
     * @param [in] ratio double ratio
     * @return error boundary
     */
    int errorBoundary(double ratio) const;

    /*!
     * @brief Method for ignoring first n frequncies
     * @details Method returns the maximal frequency if the first n frequencies are ignored
     *
     * @param [in] n number of frequencies ignored
     * @return maximal frequency
     */
    int ignoreMode(size_t n) const;

    /*!
     * @brief Method for histogram conversion
     * @details Method adds frequencies from histogram_ to a vector
     *
     * @param [out] dst histogram vector
     * @param [in] max maximal frequeny added
     */
    void toCountVector(std::vector<int>& dst, int max) const;

private:

    std::map<int, int> histogram_;
};

/*!
 * @brief Method for read correction
 * @details Method corrects reads in parralel based on k-mer frequencies. Each k-mer of a
 * read is checked if it occurres more than c times in the whole data set and if so
 * it is called a solid k-mer. All bases that aren't covered by a solid k-mer need
 * correction. A read is corrected if there exists a set of operations so that all
 * erroneous bases are corrected, otherwise it is left intact. It uses the EnhancedSuffixArray
 * to get number of occurences for each k-mer.
 *
 * @param [in] reads vector of Read object pointers
 * @param [in] k k-mer length (if -1 it is learned from reads)
 * @param [in] c threshold for solid k-mers (if -1 it is learned from reads)
 * @param [in] path path to file where the EnhancedSuffixArray objects are cached to speed up
 * future runs on the same data
 */
bool correctReads(std::vector<Read*>& reads, int k, int c, int threadLen, const char* path);

/*!
 * @brief Method for duplicate read filtering
 * @details If a read has duplicates it is picked only once and its coverage is increased by
 * the number of duplicates. EnhancedSuffixArray is used to efficiently find duplicates.
 *
 * @param [out] dst vector of non duplicate Read object pointers
 * @param [in] reads vector of Read object pointers
 * @param [in] view if true Read objects are not cloned to dst
 */
bool filterReads(std::vector<Read*>& dst, std::vector<Read*>& reads, bool view = true);
