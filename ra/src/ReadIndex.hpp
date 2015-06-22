/*!
 * @file ReadIndex.hpp
 *
 * @brief ReadIndex class header file
 * @details Algorithms were rewritten to c++ from following papers: \
 *     1. Title: Replacing suffix trees with enhanced suffix arrays \n
 *        Authors: Mohamed Ibrahim Abouelhoda, Stefan Kurtz, Enno Ohlebusch \n
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 02, 2015
 */

#pragma once

#include "IO.hpp"
#include "Read.hpp"
#include "EnhancedSuffixArray.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief ReadIndex class
 * @details Wrapper for EnhancedSuffixArray objects which helps to mantain
 * the memory complexity at 13n and also implements patter search methods.
 */
class ReadIndex {
public:

    /*!
     * @brief ReadIndex consructor
     * @details Creates a ReadIndex object from reads (or their reverse complements).
     * It concatenates the reads together puting % at the begining and #$$$$ at the end
     * of each read to obtain strings for EnhancedSuffixArray construction. After the
     * construction each $$$$ string is replaced by corresponding read identifier. If reads
     * exceed 2GB then they are split into 2GB fragments and more EnhancedSuffixArray
     * objects are created.
     *
     * @param [in] read vector of Read object poiters
     * @param [in] rk if true reverse complements are used
     */
    ReadIndex(const std::vector<Read*>& reads, int rk = 0);

    /*!
     * @brief ReadIndex destructor
     */
    ~ReadIndex();

    /*!
     * @brief Method for number of occurences retrieval
     * @details For a given pattern the method returns the number of occurences in all
     * EnhancedSuffixArray objects (complexity: O(m))
     *
     * @param [in] pattern query string
     * @param [in] m pattern length
     * @return number of occurences
     */
    size_t numberOfOccurrences(const char* pattern, int m) const;

    /*!
     * @brief Method for duplicates search
     * @details Method finds all duplicates for the given read (complexity: O(m + z)
     * where m is the length of the read and z is the number of occurences)
     *
     * @param [out] dst vector of duplicates identifiers
     * @param [in] read Read object pointer
     */
    void readDuplicates(std::vector<int>& dst, const Read* read) const;

    /*!
     * @brief Method for prefix suffix matches search
     * @details Method returns all prefix suffix matches between the query read and all
     * reads in all EhancedSuffixArray objects. Only matches with length longer than the
     * minimal provided (complexity: O(m + z) where m is the length of the read
     * and z is the number of matches)
     *
     * @param [out] dst vector of match pairs (identifier, length)
     * @param [in] read Read object pointer
     * @param [in] rk if 1 the reverse complement of read is used
     * @param [in] minOverlapLen only matches with longer length are reported
     */
    void readPrefixSuffixMatches(std::vector<std::pair<int, int>>& dst, const Read* read,
        int rk, int minOverlapLen) const;

    /*!
     * @brief Method for object size retrieval
     * @details Method returns the objects size in bytes needed for serialization.
     *
     * @return size in bytes
     */
    size_t sizeInBytes() const;

    /*!
     * @brief Method for object serialization
     * @details Method serializes the object to a byte buffer.
     *
     * @param [out] bytes byte buffer
     * @param [out] bytesLen output byte guffer length
     */
    void serialize(char** bytes, size_t* bytesLen) const;

    /*!
     * @brief Method for object deserialization
     * @details Method deserializes the object from a byte buffer.
     *
     * @param [in] bytes byte buffer
     * @return ReadIndex object
     */
    static ReadIndex* deserialize(char* bytes);

    /*!
     * @brief Method for object caching
     * @details Method caches the object to specified path.
     *
     * @param [in] path path to file for caching
     */
    void store(const char* path) const;

    /*!
     * @brief Method for cached object input
     * @details Method creates a ReadIndex object from path
     *
     * @param [in] path path to file where the object is stored
     */
    static ReadIndex* load(const char* path);

private:

    /*!
     * @brief Private ReadIndex constructor
     * @details Creates an empty ReadIndex object needed for deserialize method.
     */
    ReadIndex() {}

    /*!
     * @brief Method for interval search
     * @details Method returns a interval where all suffixes share the prefix which
     * equals the query pattern of length m (complexity: O(m)).
     * If no such subinterval exists then (s, e) = (-1, -1).
     *
     * @param [out] s interval start position
     * @param [out] e interval end position
     * @param [in] fragment identifier of EnhancedSuffixArray fragment
     * @param [in] pattern query string
     * @param [in] m pattern length
     */
    void findInterval(int* s, int* e, int fragment, const char* pattern, int m) const;

    /*!
     * @brief Method for fragment update
     * @details Method updates the EnahcedSuffixArray fragment by replacing all $$$$ strings
     * with corresponding read identifiers.
     *
     * @param [in] fragment identifier of EnhancedSuffixArray fragment
     * @param [in] start starting read which identifier is to be incorporated
     * @param [in] end ending read which identifier is to be incorporated
     * @param [in] reads vector of Read object pointers
     */
    void updateFragment(int fragment, int start, int end, const std::vector<Read*>& reads);

    int n_;
    std::vector<int> fragmentSizes_;
    std::vector<EnhancedSuffixArray*> fragments_;
};
