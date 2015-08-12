
#pragma once

#include "StringGraph.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief ContigPart class
 */
class ContigPart {
public:
  ContigPart(int src, int clr_lo, int clr_hi, int offset) :
    src(src), clr_lo(clr_lo), clr_hi(clr_hi), offset(offset) {}

  /*!
   * @brief Source read id
   */
  int src;

  /*!
   * @brief Index of first used base from source read. 
   */
  int clr_lo;

  /*!
   * @brief Index of last used base from source read. 
   */
  int clr_hi;

  /*!
   * @brief Offset in sequence 
   */
  int offset;

  /*!
   * @brief Contig part type
   * @details normal 0, rk 1
   */
  int type() const {
    return clr_hi < clr_lo;
  }
};

/*!
 * @brief Contig class
 */
class Contig {
public:

    /*!
     * @brief Contig constructor
     * @details Creates an empty Contig object
     */
    Contig() {}

    /*!
     * @brief Contig constructor
     * @details Creates a Contig object from SringGraphWalk object
     *
     * @param [in] walk StrinGraphWalk object pointer
     */
    Contig(const StringGraphWalk* walk);

    /*!
     * @brief Contig destructor
     */
    ~Contig() {}

    /*!
     * @brief Getter for contig parts
     * @return parts
     */
    const std::vector<ContigPart>& getParts() const {
        return parts_;
    }

    /*!
     * @brief Method for adding parts
     *
     * @param [in] part part tuple to be added to parts
     */
    void addPart(const ContigPart& part) {
        parts_.emplace_back(part);
    }

private:

    std::vector<ContigPart> parts_;
};
