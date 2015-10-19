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
#include "Overlap.hpp"
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
    void store_reads(const ReadSet& src);

    /*!
     * @bried Method for loading a single Read object stored beforehand
     * @details Loads a Read object from a binary file in the depot folder
     * determined by the given index
     *
     * @param [in] index index of the wanted Read object
     * @return Read object pointer
     */
    Read* load_read(uint32_t index);

    /*!
     * @brief Method for loading the set of Read objects stored beforehand
     * @details Loads Read objects from a binary file in the depot folder
     *
     * @param [out] dst set of Read object pointers
     */
    void load_reads(ReadSet& dst);

    /*!
     * @brief Method for loading an incomplete set of Read objects stored beforehand
     * @details Loads the given number of Read objects from a binary file in
     * the depot folder starting from the given index
     *
     * @param [out] dst set of Read object pointers
     * @param [in] begin index of first Read object
     * @param [in] length length of Read objects to be loaded (if length goes
     * out of range, function returns all objects from the beginning index
     * to the last object available)
     */
    void load_reads(ReadSet& dst, uint32_t begin, uint32_t length);

    /*!
     * @brief Method for storing Overlap objects
     * @details Stores overlap objects to a binary file in the depot folder
     *
     * @param [in] src set of Overlap object pointers
     */
    void store_overlaps(const OverlapSet& src);

    /*!
     * @bried Method for loading a single Overlap object stored beforehand
     * @details Loads a Overlap object from a binary file in the depot folder
     * determined by the given index
     *
     * @param [in] index index of the wanted Overlap object
     * @return Overlap object pointer
     */
    Overlap* load_overlap(uint32_t index);

    /*!
     * @brief Method for loading the set of Overlap objects stored beforehand
     * @details Loads Overlap objects from a binary file in the depot folder
     *
     * @param [out] dst set of Overlap object pointers
     */
    void load_overlaps(OverlapSet& dst);

    /*!
     * @brief Method for loading an incomplete set of Overlap objects stored beforehand
     * @details Loads the given number of Overlap objects from a binary file in
     * the depot folder starting from the given index
     *
     * @param [out] dst set of Overlap object pointers
     * @param [in] begin index of first Overlap object
     * @param [in] length length of Overlap objects to be loaded (if length goes
     * out of range, function returns all objects from the beginning index
     * to the last object available)
     */
    void load_overlaps(OverlapSet& dst, uint32_t begin, uint32_t length);

private:

    template<typename T>
    void store(const std::vector<T*>& src, FILE* data, FILE* index);

    template<typename T>
    void load(std::vector<T*>& dst, uint32_t begin, uint32_t length,
        FILE* data, FILE* index);

    std::mutex mutex_;

    FILE* read_data_;
    FILE* read_index_;
    FILE* overlap_data_;
    FILE* overlap_index_;
};
