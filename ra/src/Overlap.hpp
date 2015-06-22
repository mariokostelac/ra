/*!
 * @file Overlap.hpp
 *
 * @brief Overlap class header file
 * @details
 * Filtering of contained and transitive overlaps was rewritten and modified from:
 *     Github: https://github.com/mariokostelac/assembly-tools \n
 * Overlap types:
 *     (found at sourceforge.net/p/amos/mailman/message/19965222/) \n
 *
 *     Normal: \n\n
 *
 *     read a  ---------------------------------->   bHang     \n
 *     read b      aHang  -----------------------------------> \n\n
 *
 *     read a     -aHang  -----------------------------------> \n
 *     read b  ---------------------------------->  -bHang     \n\n
 *
 *     Innie: \n\n
 *
 *     read a  ---------------------------------->   bHang     \n
 *     read b      aHang  <----------------------------------- \n\n
 *
 *     read a     -aHang  -----------------------------------> \n
 *     read b  <----------------------------------  -bHang     \n\n
 *
 *     Containment: \n\n
 *
 *     read a  ----------------------------------------------> \n
 *     read b      aHang  ----------------------->  -bHang     \n\n
 *
 *     read a     -aHang  ----------------------->   bHang     \n
 *     read b  ----------------------------------------------> \n\n
 *
 *     (same if read b is reversed)
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 14, 2015
 */

#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief Overlap class
 */
class Overlap {
public:

    /*!
     * @brief Overlap constructor
     * @details Creates an Overlap object from read ids, overlap length, read hangs and
     * overlap type.
     *
     * @param [in] a identifier of read A
     * @param [in] b identifier of read B
     * @param [in] length length of overlap between reads
     * @param [in] aHang hang of read A
     * @param [in] bHang hang of read B
     * @param [in] innie true if overlap type is innie
     */
    Overlap(int a, int b, int length, int aHang, int bHang, bool innie);

    /*!
     * @brief Overlap destructor
     */
    ~Overlap() {}

    /*!
     * @brief Getter for read A identifier
     * @return read A identifier
     */
    int getA() const {
        return a_;
    }

    /*!
     * @brief Getter for read B identifier
     * @return read B identifier
     */
    int getB() const {
        return b_;
    }

    /*!
     * @brief Getter for overlap length
     * @return length
     */
    int getLength() const {
        return length_;
    }

    /*!
     * @brief Getter for read A hang
     * @return read A hang
     */
    int getAHang() const {
        return aHang_;
    }

    /*!
     * @brief Getter for read B hang
     * @return read B hang
     */
    int getBHang() const {
        return bHang_;
    }

    /*!
     * @brief Getter for overlap type
     * @return true if overlap is innie
     */
    bool isInnie() const {
        return innie_;
    }

    /*!
     * @brief Method for prefix check
     * @detals Method checks whether the start of the read is contained in overlap.
     * It respects the direction of read (important for reverse complements)!
     *
     * @param [in] readId read identifier
     * @return true if prefix is in overlap
     */
    bool isUsingPrefix(int readId) const;

    /*!
     * @brief Method for suffix check
     * @detals Method checks whether the end of the read is contained in overlap.
     * It respects the direction of read (important for reverse complements)!
     *
     * @param [in] readId read identifier
     * @return true if suffix is in overlap
     */
    bool isUsingSuffix(int readId) const;

    /*!
     * @brief Method for transitive overlap check
     * @details Method checks whether this (o1) is transitive considering overlaps o2 and o3.
     *
     * @param [in] o2 overlap 2
     * @param [in] o3 overlap 3
     * @return true if this is transitive
     */
    bool isTransitive(const Overlap* o2, const Overlap* o3) const;

    /*!
     * @brief Getter for hanging length of a read
     * @details Method returns the absolute value of the read hang.
     *
     * @param [in] readId read identifier
     * @return hanging length
     */
    uint hangingLength(int readId) const;

    /*!
     * @brief Method for object cloning
     *
     * @return new Overlap object which equals this
     */
    Overlap* clone() const;

    // updates overlap ids to match read->getId()
    /*!
     * @brief Method for overlap update
     * @details Method updates read identifiers in overlap to match real read identifiers.
     * This method will be removed in near future!
     *
     * @param [in] overlaps vector of Overlap objets pointers
     * @param [in] reads vector of Read objets pointers
     */
    friend void updateOverlapIds(std::vector<Overlap*>& overlaps, std::vector<Read*>& reads);

private:

    int a_;
    int b_;
    int length_;
    int aHang_;
    int bHang_;
    bool innie_;
};

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
void overlapReads(std::vector<Overlap*>& dst, std::vector<Read*>& reads, int minOverlapLen,
    int threadLen, const char* path);

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