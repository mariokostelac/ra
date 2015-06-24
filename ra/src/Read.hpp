/*!
 * @file Read.hpp
 *
 * @brief Read class header file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Apr 20, 2015
 */

#pragma once

#include "CommonHeaders.hpp"

/*!
 * @brief Read class
 */
class Read {
public:

    /*!
     * @brief Read destructor
     */
    virtual ~Read() {};

    /*!
     * @brief Getter for identifier
     * @return identifier
     */
    virtual int getId() const = 0;

    /*!
     * @brief Getter for name
     * @return name
     */
    virtual const std::string& getName() const = 0;

    /*!
     * @brief Getter for sequence
     * @return sequence
     */
    virtual const std::string& getSequence() const = 0;

    /*!
     * @brief Getter for length
     * @return length
     */
    virtual size_t getLength() const = 0;

    /*!
     * @brief Getter for quality
     * @return quality
     */
    virtual const std::string& getQuality() const = 0;

    /*!
     * @brief Getter for reverse complement
     * @return reverse complement
     */
    virtual const std::string& getReverseComplement() const = 0;

    /*!
     * @brief Getter for coverage
     * @return coverage
     */
    virtual double getCoverage() const = 0;

    /*!
     * @brief Setter for coverage
     * @details Adds value to coverage
     *
     * @param [in] value increasing coverage value
     */
    virtual void addCoverage(double value) = 0;

    /*!
     * @brief Method for object cloning
     *
     * @return new Read object which equals this
     */
    virtual Read* clone() const = 0;

    /*!
     * @brief Method for sequence correction
     * @details Method changes characted at position idx to character c.
     *
     * @param [in] idx position in sequence
     * @param [in] c new character
     */
    virtual void correctBase(int idx, int c) = 0;

    /*!
     * @brief Construction of the reverse complement
     * @details Method constructs a reverse complement from the sequence.
     * and stores it in the object
     */
    virtual void createReverseComplement() = 0;
};

/*!
 * @brief Method for parallel reverse complement construction
 * @details Method constructs reverse complements for each read provided
 * in parralel.
 *
 * @param [in] reads vector of Read objects pointers
 * @param [in] threadLen number of threads
 */
void createReverseComplements(std::vector<Read*>& reads, int threadLen);
