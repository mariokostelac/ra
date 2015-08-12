
#pragma once

#include "StringGraph.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief Contig class
 */
class Contig {
public:

    /*!
     * @brief Part tuple
     * @details Part tuple = read id, type: normal 0 - rk 1, offset, lo, hi.
     * It is used for writig contigs in afg format.
     */
    typedef std::tuple<int, int, int, int, int> Part;

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
    const std::vector<Part>& getParts() const {
        return parts_;
    }

    /*!
     * @brief Method for adding parts
     *
     * @param [in] part part tuple to be added to parts
     */
    void addPart(const Part& part) {
        parts_.emplace_back(part);
    }

private:

    std::vector<Part> parts_;
};
