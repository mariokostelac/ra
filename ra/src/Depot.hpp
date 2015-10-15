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
     * @brief Method for storing Read objects
     * @details Stores Read objects to a binary file in the depot folder
     *
     * @param [in] src set of Read object pointers
     */
    void store_reads(const ReadSet& src) const;

    /*!
     * @brief Method for loading the set of Read objects stored beforehand
     * @details Loads Read objects from a binary file in the depot folder
     *
     * @param [out] dst set of Read object pointers
     */
    void load_reads(ReadSet& dst) const;

    /*!
     * @brief Method for loading an incomplete set of Read objects stored beforehand
     * @details Loads the given number of Read objects from a binary file in
     * the depot folder starting from the given index
     *
     * @param [out] dst set of Read object pointers
     * @param [in] begin index of first Read object
     * @param [in] length length of Read objects to be loaded
     */
    void load_reads(ReadSet& dst, uint32_t begin, uint32_t length) const;

    /*!
     * @bried Method for loading a single Read object stored beforehand
     * @details Loads a Read object from a binary file in the depot folder
     * determined by the given index
     *
     * @param [in] index index of the wanted Read object
     * @return Read object pointer
     */
    Read* load_read(uint32_t index) const;

private:

    FILE* reads_data_;
    FILE* reads_index_;
    FILE* overlaps_data_;
    FILE* overlaps_index_;
};
