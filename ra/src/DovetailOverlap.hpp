/*!
 * @file DovetailOverlap.hpp
 *
 * @brief DovetailOverlap class header file
 * @details
 * Filtering of contained and transitive overlaps was rewritten and modified from:
 *     Github: https://github.com/mariokostelac/assembly-tools \n
 * DovetailOverlap types:
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
#include "Overlap.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief DovetailOverlap class
 */
class DovetailOverlap : public Overlap {
public:

    /*!
     * @brief DovetailOverlap constructor
     */
    DovetailOverlap(const int a_id, const int b_id, const int a_hang, const int b_hang, const bool innie)
      : Overlap(a_id, b_id, innie) {
        a_hang_ = a_hang;
        b_hang_ = b_hang;
    }

    /*!
     * @brief DovetailOverlap constructor
     */
    DovetailOverlap(const int a_id, Read* a, const int b_id, Read* b, const bool innie):
      Overlap(a_id, a, b_id, b, innie) {}

    /*!
     * @brief DovetailOverlap destructor
     */
    virtual ~DovetailOverlap() {}

    /*!
     * @brief Getter for overlap length
     * @return length
     */
    virtual int length() const = 0;

    /*!
     * @brief Getter for overlap length in given read
     *
     * @param [in] readId read identifier
     * @return length
     */
    virtual int length(int read_id) const = 0;

    /*!
     * @brief Getter for read A hang
     * @return read A hang
     */
    int a_hang() const {
      return a_hang_;
    }

    /*!
     * @brief Getter for read B hang
     * @return read B hang
     */
    int b_hang() const {
      return b_hang_;
    }

    /*!
     * @brief Method for prefix check
     * @detals Method checks whether the start of the read is contained in overlap.
     * It respects the direction of read (important for reverse complements)!
     *
     * @param [in] readId read identifier
     * @return true if prefix is in overlap
     */
    bool is_using_prefix(int readId) const;

    /*!
     * @brief Method for suffix check
     * @detals Method checks whether the end of the read is contained in overlap.
     * It respects the direction of read (important for reverse complements)!
     *
     * @param [in] readId read identifier
     * @return true if suffix is in overlap
     */
    bool is_using_suffix(int readId) const;

    /*!
     * @brief Method for transitive overlap check
     * @details Method checks whether this (o1) is transitive considering overlaps o2 and o3.
     *
     * @param [in] o2 overlap 2
     * @param [in] o3 overlap 3
     * @return true if this is transitive
     */
    bool is_transitive(const DovetailOverlap* o2, const DovetailOverlap* o3) const;

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
     * @return new DovetailOverlap object which equals this
     */
    virtual DovetailOverlap* clone() const = 0;


    /*!
     * @brief Method for overlap representation
     * @details Method prints overlap representation to the given stream.
     *
     * @param [in] output stream.
     */
    virtual void print(std::ostream& str) const = 0;

    /*!
     * @brief Operator prints overlap representation.
     * @details Operator prints given overlap representation to the given stream.
     *
     * @param [in] output stream.
     * @param [in] overlap.
     */
    friend std::ostream& operator<<(std::ostream& str, DovetailOverlap const& data)
    {
      data.print(str);
      return str;
    }

    std::string repr() const;

protected:

    int a_hang_;
    int b_hang_;
};

/*!
 * @brief Method for overlaping reads
 * @details Method creates EnhancesSuffixArray objects from reads and uses them for pattern
 * matching, i.e. prefix-sufix overlaps. It also creates reverse complements of reads needed
 * to get all types of overlaps.
 *
 * @param [out] dst vector of DovetailOverlap objects pointers
 * @param [in] reads vector of Read objects pointers
 * @param [in] minDovetailOverlapLen minimal length of overlaps considered
 * @param [in] threadLen number of threads
 * @param [in] path path to file where the EnhancedSuffixArray objects are cached to speed up
 * future runs on the same data
 */
void overlapReads(std::vector<DovetailOverlap*>& dst, std::vector<Read*>& reads, int minDovetailOverlapLen,
    int threadLen, const char* path);

