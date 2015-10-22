/*!
 * @file Read.hpp
 *
 * @brief Read class header file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Apr 20, 2015
 */

#pragma once

#include "DepotObject.hpp"
#include "CommonHeaders.hpp"

class Read;
using ReadSet = std::vector<Read*>;

/*!
 * @brief Read class
 */
class Read: public DepotObject {
public:

    Read(uint32_t id, const std::string& name, const std::string& sequence,
        const std::string& quality, double coverage);

    /*!
     * @brief Read destructor
     */
    ~Read() {};

    /*!
     * @brief Getter for identifier
     * @return identifier
     */
    uint32_t id() const {
        return id_;
    }

    /*!
     * @brief Getter for name
     * @return name
     */
    const std::string& name() const {
        return name_;
    }

    /*!
     * @brief Getter for sequence
     * @return sequence
     */
    const std::string& sequence() const {
        return sequence_;
    }

    /*!
     * @brief Getter for length
     * @return length
     */
    uint32_t length() const {
        return sequence_.size();
    }

    /*!
     * @brief Getter for quality
     * @return quality
     */
    const std::string& quality() const {
        return quality_;
    }

    /*!
     * @brief Getter for reverse complement
     * @return reverse complement
     */
    const std::string& reverse_complement() const {
        return reverse_complement_;
    }

    /*!
     * @brief Construction of the reverse complement
     * @details Method constructs a reverse complement from the sequence.
     * and stores it in the object
     */
    void create_reverse_complement();

    /*!
     * @brief Getter for coverage
     * @return coverage
     */
    double coverage() const {
        return coverage_;
    }

    /*!
     * @brief Setter for coverage
     * @details Adds value to coverage
     *
     * @param [in] value increasing coverage value
     */
    void add_coverage(double value) {
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
    void correct_base(uint32_t idx, char c);

    /*!
     * @brief Method for object serialization
     * @details Serializes the object to a char array
     *
     * @param [out] bytes adress of the char array where the object is serialized
     * @param [out] bytes_length adress of the variable holding size of the
     * serialized object
     */
    void serialize(char** bytes, uint32_t* bytes_length) const;

    /*!
     * @brief Method for object deserialization
     * @details Deserializes an object stored in a char array
     *
     * @param [in] bytes char array where the object was serialized
     * @return Read object pointer
     */
    static Read* deserialize(const char* bytes);

private:

    Read() {};

    static DepotObjectType type_;

    uint32_t id_;
    std::string name_;
    std::string sequence_;
    std::string quality_;
    double coverage_;
    std::string reverse_complement_;
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
