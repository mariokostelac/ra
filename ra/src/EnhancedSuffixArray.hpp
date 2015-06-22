/*!
 * @file EnhancedSuffixArray.hpp
 *
 * @brief EnhancedSuffixArray class header file
 * @details Header file with declaration of EnhancedSuffixArray class and its methods.\n
 * Algorithms were rewritten to c++ from following papers:\n
 *     1. Title: Two efficient algorithms for linear time suffix array construction\n
 *        Authors: Ge Nong, Sen Zhang, Wai Hong Chan\n
 *     2. Title: Replacing suffix trees with enhanced suffix arrays\n
 *        Authors: Mohamed Ibrahim Abouelhoda, Stefan Kurtz, Enno Ohlebusch\n
 *     3. Title: Linear-time longest-common-prefix computation in suffix arrays and its applications\n
 *        Authors: Toru Kasai, Gunho Lee, Hiroki Arimura, Setsuo Arikawa, Kunsoo Park\n
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Apr 18, 2015
 */

#pragma once

#include "CommonHeaders.hpp"

/*!
 * @brief EnhancedSuffixArray class
 * @details Enhaced suffix array = suffix array + longest common prefix table +
 * child table (+ other tables which are not needed here).
 * Class is used mostly for pattern search.
 */
class EnhancedSuffixArray {
public:

    /*!
     * @brief EnhancedSuffixArray constructor
     * @details Creates EnhancedSuffixArray object from a given string which include
     * construction of the suffix array, longest common prefix table and child table.
     *
     * @param [in] str string
     */
    EnhancedSuffixArray(const std::string& str);

    /*!
     * @brief EnhancedSuffixArray destructor
     */
    ~EnhancedSuffixArray() {}

    /*!
     * @brief Getter for stored sequence length
     * @return length
     */
    int getLength() const {
        return n_;
    }

    /*!
     * @brief Getter for the stored sequence
     * @return sequence
     */
    const std::string& getString() const {
        return str_;
    }

    /*!
     * @brief Getter for suffix start position
     * @details Getter returns the i-th suffix from the suffix array.
     *
     * @param [in] i position in suffix array
     * @return i-th suffix start position
     */
    int getSuffix(int i) const {
        ASSERT(i >= 0 && i < n_, "ESA", "index out of range");
        return suftab_[i];
    }

    /*!
     * @brief Method for subinterval search by character
     * @details Method returns a subinterval of the given l-interval [i, j]
     * which has the character c at positin l (complexity: O(1)).
     * If no such subinterval exists then (s, e) = (-1, -1).
     *
     * @param [out] s subinterval start position
     * @param [out] e subinterval end position
     * @param [in] i interval start position
     * @param [in] j interval end position
     * @param [in] c character
     */
    void intervalSubInterval(int* s, int* e, int i, int j, char c) const;

    /*!
     * @brief Method for interval longest common prefix length retrieval
     * @details Method returns the lenght of the longest common prefix of all
     * suffixes in the given interval [i, j] (complexity: O(1)).
     *
     * @param [in] i interval start position
     * @param [in] j interval end position
     * @return logest common prefix lenght
     */
    int intervalLcpLen(int i, int j) const;

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
     * @param [out] bytes output byte buffer
     * @param [out] bytesLen output byte guffer length
     */
    void serialize(char** bytes, size_t* bytesLen) const;

    /*!
     * @brief Method for object deserialization
     * @details Method deserializes the object from a byte buffer.
     *
     * @param [int] bytes byte buffer
     * @\return EnhancedSuffixArray object
     */
    static EnhancedSuffixArray* deserialize(const char* bytes);

    /*!
     * @brief Method for object printing
     * @details Method prints the object to stdout in tabular format as
     * id, suftab, lcptab, childtab, suffix.
     */
    void print() const;

    friend class ReadIndex;

private:

    /*!
     * @brief Private EnhancedSuffixArray constructor
     * @details Creates an empty EnhancedSuffixArray object needed for deserialize method
     */
    EnhancedSuffixArray() {};

    /*!
     * @brief Method for suffix array creation
     * @details Called by the EnhacedSuffixArray public constructor to create the
     * suffix array with the SA-IS algorithm which is based on induced sorting
     * (article [1], complexity: O(n))
     *
     * @param [in] s pointer to suffix array
     * @param [in] n length of suffix array
     * @param [in] csize size of elements in suffix array
     * @param [in] alphabetSize number of different elements in input string
     */
    void createSuffixArray(const unsigned char* s, int n, int csize, int alphabetSize = 256);

    /*!
     * @brief Method for longest common prefix table creation
     * @details Called by the EnhacedSuffixArray public constructor to create the
     * longest common prefix table from informatin in suffix array
     * (article [2], complexity: O(n))
     */
    void createLongestCommonPrefixTable();

    /*!
     * @brief Method for child table creation
     * @details Called by the EnhacedSuffixArray public constructor to create the
     * child table from information in longest common prefix table
     * (article [3], complexity: O(n))
     */
    void createChildTable();

    int n_;
    std::string str_;
    std::vector<int> suftab_;
    std::vector<int> lcptab_;
    std::vector<int> childtab_;
};
