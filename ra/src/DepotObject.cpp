/*!
 * @file DepotObject.cpp
 *
 * @brief DepotObject class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Oct 19, 2015
 */

#include "Read.hpp"
#include "Overlap.hpp"
#include "DepotObject.hpp"

DepotObjectType DepotObject::type_ = DepotObjectType::kDefault;

DepotObject* DepotObject::deserialize(const char* bytes) {

    DepotObjectType type;
    std::memcpy(&type, bytes, sizeof(type));

    switch (type) {
        case DepotObjectType::kRead:
            return static_cast<DepotObject*>(Read::deserialize(bytes));
        case DepotObjectType::kOverlap:
            return static_cast<DepotObject*>(Overlap::deserialize(bytes));
        default:
            ASSERT(false, "DepotObject", "Not supported objet type!");
    }
}
