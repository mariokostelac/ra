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
    Overlap(const int a_id, const int b_id, const bool innie)
      : a_id_(a_id), b_id_(b_id), innie_(innie) {}

    /*!
     * @brief Overlap constructor
     */
    Overlap(const int a_id, Read* read_a, const int b_id, Read* read_b, const bool innie):
      a_id_(a_id), read_a_(read_a), b_id_(b_id), read_b_(read_b), innie_(innie) {}

    /*!
     * @brief Overlap destructor
     */
    virtual ~Overlap() {}

    /*!
     * @brief Getter for read A identifier
     * @return read A identifier
     */
    int getA() const {
      return a_id_;
    }

    /*!
     * @brief Getter for read A
     * @return read A
     */
    Read* getReadA() const {
      return read_a_;
    }

    /*!
     * @brief Setter for read A
     * @return
     */
    void setReadA(Read* read) {
      read_a_ = read;
    }

    /*!
     * @brief Getter for read B identifier
     * @return read B identifier
     */
    int getB() const {
      return b_id_;
    }

    /*!
     * @brief Getter for read B
     * @return read B
     */
    Read* getReadB() const {
      return read_b_;
    }

    /*!
     * @brief Setter for read B identifier
     * @return
     */
    void setReadB(Read* read) {
      read_b_ = read;
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

    int a_id_;
    Read* read_a_;

    int b_id_;
    Read* read_b_;

    bool innie_;
};

