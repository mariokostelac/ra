/*!
 * @file Depot.hpp
 *
 * @brief Depot class header file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Oct 14, 2015
 */

#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief Depot class
 */
class Depot {
public:

    /*!
     * @brief Depot constructor
     * @details Creates a Depot object from a path to the wanted folder
     *
     * @param [in] path path to a folder
     */
    Depot(const std::string& path);

    /*!
     * @brief Depot destructor
     */
    ~Depot();

    /*!
     * @brief Method for storing a set of Read objects
     */
    void store_reads(const ReadSet& src) const;

    /*!
     * @brief Method for loading the set of Read objects stored beforehand
     */
    void load_reads(ReadSet& dst) const;

    /*!
     * @brief Method for loading a part of the set of Read objects stored beforehand
     */
    void load_reads(ReadSet& dst, uint32_t start, uint32_t end);

private:

    FILE* reads_data_;
    FILE* reads_index_;
    FILE* overlaps_data_;
    FILE* overlaps_index_;
};
