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
     */
    Overlap(const int read_id_a, const int read_id_b): a_(read_id_a), b_(read_id_b) {}

    /*!
     * @brief Overlap constructor
     */
    Overlap(const int read_id_a, Read* read_a, const int read_id_b, Read* read_b):
      a_(read_id_a), ra_(read_a), b_(read_id_b), rb_(read_b) {}

    /*!
     * @brief Overlap destructor
     */
    virtual ~Overlap() {}

    /*!
     * @brief Getter for read A identifier
     * @return read A identifier
     */
    int getA() const {
      return a_;
    }

    /*!
     * @brief Setter for read A identifier
     * @return
     */
    void setA(int read_id) {
      if (read_id != a_) {
        debug("OVLCHID %d %d A\n", a_, read_id);
      }
      a_ =  read_id;
    }

    /*!
     * @brief Getter for read A
     * @return read A
     */
    Read* getReadA() const {
      return ra_;
    }

    /*!
     * @brief Setter for read A
     * @return
     */
    void setReadA(Read* read) {
      ra_ = read;
    }

    /*!
     * @brief Getter for read B identifier
     * @return read B identifier
     */
    int getB() const {
      return b_;
    }

    /*!
     * @brief Setter for read B identifier
     * @return
     */
    void setB(int read_id) {
      if (read_id != b_) {
        debug("OVLCHID %d %d B\n", b_, read_id);
      }
      b_ = read_id;
    }

    /*!
     * @brief Getter for read B
     * @return read B
     */
    Read* getReadB() const {
      return rb_;
    }

    /*!
     * @brief Setter for read B identifier
     * @return
     */
    void setReadB(Read* read) {
      rb_ = read;
    }

    /*!
     * @brief Getter for overlap length
     * @return length
     */
    virtual int getLength() const = 0;

    /*!
     * @brief Getter for overlap length in given read
     *
     * @param [in] readId read identifier
     * @return length
     */
    virtual int getLength(int read_id) const = 0;

    /*!
     * @brief Getter for read A hang
     * @return read A hang
     */
    virtual int getAHang() const = 0;

    /*!
     * @brief Getter for read B hang
     * @return read B hang
     */
    virtual int getBHang() const = 0;

    /*!
     * @brief Getter for overlap score 
     * @return overlap score
     */
    virtual int getScore() const = 0;

    /*!
     * @brief Getter for overlap type
     * @return true if overlap is innie
     */
    virtual bool isInnie() const = 0;

    /*!
     * @brief Method for prefix check
     * @detals Method checks whether the start of the read is contained in overlap.
     * It respects the direction of read (important for reverse complements)!
     *
     * @param [in] readId read identifier
     * @return true if prefix is in overlap
     */
    virtual bool isUsingPrefix(int readId) const = 0;

    /*!
     * @brief Method for suffix check
     * @detals Method checks whether the end of the read is contained in overlap.
     * It respects the direction of read (important for reverse complements)!
     *
     * @param [in] readId read identifier
     * @return true if suffix is in overlap
     */
    virtual bool isUsingSuffix(int readId) const = 0;

    /*!
     * @brief Method for transitive overlap check
     * @details Method checks whether this (o1) is transitive considering overlaps o2 and o3.
     *
     * @param [in] o2 overlap 2
     * @param [in] o3 overlap 3
     * @return true if this is transitive
     */
    virtual bool isTransitive(const Overlap* o2, const Overlap* o3) const;

    /*!
     * @brief Getter for hanging length of a read
     * @details Method returns the absolute value of the read hang.
     *
     * @param [in] readId read identifier
     * @return hanging length
     */
    virtual uint hangingLength(int readId) const = 0;

    /*!
     * @brief Method for object cloning
     *
     * @return new Overlap object which equals this
     */
    virtual Overlap* clone() const = 0;

    /*!
     * @brief Method for overlap update
     * @details Method updates read identifiers in overlap to match real read identifiers.
     * This method will be removed in near future!
     *
     * @param [in] overlaps vector of Overlap objets pointers
     * @param [in] reads vector of Read objets pointers
     */
    friend void updateOverlapIds(std::vector<Overlap*>& overlaps, std::vector<Read*>& reads);


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
    friend std::ostream& operator<<(std::ostream& str, Overlap const& data)
    {
      data.print(str);
      return str;
    }

    std::string repr() const;

private:

    int a_;
    Read* ra_;

    int b_;
    Read* rb_;
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

