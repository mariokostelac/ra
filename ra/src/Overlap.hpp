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
    Overlap(const int read_id_a, const int read_id_b, const bool innie)
      : a_(read_id_a), b_(read_id_b), innie_(innie) {}

    /*!
     * @brief Overlap constructor
     */
    Overlap(const int read_id_a, Read* read_a, const int read_id_b, Read* read_b, const bool innie):
      a_(read_id_a), ra_(read_a), b_(read_id_b), rb_(read_b), innie_(innie) {}

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
     * @brief Getter for overlap score
     * @return overlap score
     */
    virtual double getScore() const;

    /*!
     * @brief Getter for overlap quality
     * @return overlap quality
     */
    virtual double getQuality() const = 0;

    /*!
     * @brief Getter for overlap type
     * @return true if overlap is innie
     */
    bool isInnie() const {
      return innie_;
    }

    /*!
     * @brief Method for object cloning
     *
     * @return new Overlap object which equals this
     */
    virtual Overlap* clone() const = 0;

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

protected:

    int a_;
    Read* ra_;

    int b_;
    Read* rb_;

    bool innie_;
};

