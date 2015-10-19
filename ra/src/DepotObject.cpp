/*!
 * @file DepotObject.cpp
 *
 * @brief DepotObject class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Oct 19, 2015
 */

#include "AfgRead.hpp"
#include "DepotObject.hpp"

DepotObjectType DepotObject::type_ = DepotObjectType::kDefault;

DepotObject* DepotObject::deserialize(const char* bytes) {

    DepotObjectType type;
    std::memcpy(&type, bytes, sizeof(type));

    switch (type) {
        case DepotObjectType::kAfgRead:
            return static_cast<DepotObject*>(AfgRead::deserialize(bytes));
        default:
            ASSERT(false, "DepotObject", "Not supported objet type!");
    }
}
