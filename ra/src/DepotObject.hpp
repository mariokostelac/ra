/*!
 * @file DepotObject.hpp
 *
 * @brief DepotObject class header file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Oct 19, 2015
 */

#pragma once

#include "CommonHeaders.hpp"

enum class DepotObjectType {
    kRead,
    kDefault
};

/*!
 * @brief DepotObject class
 */
class DepotObject {
public:

    /*!
     * @brief DepotObject destructor
     */
    virtual ~DepotObject() {};

    /*!
     * @brief Method for object serialization
     * @details Serializes the object to a char array
     *
     * @param [out] bytes adress of the char array where the object is serialized
     * @param [out] bytes_length adrees of the variable holding size of the
     * serialized object
     */
    virtual void serialize(char** bytes, uint32_t* bytes_length) const = 0;

    /*!
     * @brief Method for object deserialization
     * @details Deserializes the object from a char array
     *
     * @param [in] type type of the object stored in bytes array
     * @param [in] bytes char array where the object is serialized
     * @return DepotObject pointer
     */
    static DepotObject* deserialize(const char* bytes);

private:

    static DepotObjectType type_;
};
