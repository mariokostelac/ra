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
     * @brief Read constructor
     * @details Creates a Read object from a sequence, its name, id, quality and coverage.
     *
     * @param [in] id sequence identifier
     * @param [in] name sequence name
     * @param [in] sequence sequence string
     * @param [in] quality sequence quality
     * @param [in] coverage sequence coverage
     */
    Read(int id, const std::string& name, const std::string& sequence, const std::string& quality, double coverage);

    /*!
     * @brief Read destructor
     */
    ~Read() {};

    /*!
     * @brief Getter for identifier
     * @return identifier
     */
    int getId() const {
        return id_;
    }

    /*!
     * @brief Getter for name
     * @return name
     */
    const std::string& getName() const {
        return name_;
    }

    /*!
     * @brief Getter for sequence
     * @return sequence
     */
    const std::string& getSequence() const {
        return sequence_;
    }

    /*!
     * @brief Getter for length
     * @return length
     */
    size_t getLength() const {
        return sequence_.size();
    }

    /*!
     * @brief Getter for quality
     * @return quality
     */
    const std::string& getQuality() const {
        return quality_;
    }

    /*!
     * @brief Getter for reverse complement
     * @return reverse complement
     */
    const std::string& getReverseComplement() const {
        return reverseComplement_;
    }

    /*!
     * @brief Getter for coverage
     * @return coverage
     */
    double getCoverage() const {
        return coverage_;
    }

    /*!
     * @brief Setter for coverage
     * @details Adds value to coverage
     *
     * @param [in] value increasing coverage value
     */
    void addCoverage(double value) {
        coverage_ += value;
    }

    /*!
     * @brief Method for object cloning
     *
     * @return new Read object which equals this
     */
    Read* clone() const;

    /*!
     * @brief Method for sequence correction
     * @details Method changes characted at position idx to character c.
     *
     * @param [in] idx position in sequence
     * @param [in] c new character
     */
    void correctBase(int idx, int c);

    /*!
     * @brief Construction of the reverse complement
     * @details Method constructs a reverse complement from the sequence.
     * and stores it in the object
     */
    void createReverseComplement();

private:

    int id_;
    std::string name_;
    std::string sequence_;
    std::string quality_;
    double coverage_;
    std::string reverseComplement_;
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
