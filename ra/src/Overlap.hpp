/*!
 * @file Overlap.hpp
 *
 * @brief Overlap class header file
 * @details
 * Overlap types:
 *     (found at sourceforge.net/p/amos/mailman/message/19965222/) \n
 *
 *     Normal: \n\n
 *
 *     read a  ---------------------------------->   bHang     \n
 *     read b      aHang  -----------------------------------> \n\n
 *
 *     read a     -aHang  -----------------------------------> \n
 *     read b  ---------------------------------->  -bHang     \n\n
 *
 *     Innie: \n\n
 *
 *     read a  ---------------------------------->   bHang     \n
 *     read b      aHang  <----------------------------------- \n\n
 *
 *     read a     -aHang  -----------------------------------> \n
 *     read b  <----------------------------------  -bHang     \n\n
 *
 *     Containment: \n\n
 *
 *     read a  ----------------------------------------------> \n
 *     read b      aHang  ----------------------->  -bHang     \n\n
 *
 *     read a     -aHang  ----------------------->   bHang     \n
 *     read b  ----------------------------------------------> \n\n
 *
 *     (same if read b is reversed, i.e. overlap is innie)
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 14, 2015
 */

#pragma once

#include "Read.hpp"
#include "DepotObject.hpp"
#include "CommonHeaders.hpp"

class Depot;
class Overlap;
using OverlapSet = std::vector<Overlap*>;

/*!
 * @brief Overlap class
 */
class Overlap: public DepotObject {
public:

    /*!
     * @brief Overlap constructor
     * @details Creates a dovetail Overlap object from Read object pointers,
     * read hangs and overlap type.
     *
     * @param [in] read_a read A pointer
     * @param [in] a_hang hang of read A
     * @param [in] read_b read B pointer
     * @param [in] b_hang hang of read B
     * @param [in] innie true if overlap type is innie
     */
    Overlap(const Read* read_a, int32_t a_hang, const Read* read_b,
        int32_t b_hang, bool is_innie, double err_rate = -1,
        double orig_err_rate = -1, uint32_t confirmations = 1);

    /*!
     * @brief Overlap constructor
     * @details Creates an Overlap object from Read object pointers, information
     * where overlap begins and ends on each of given reads and information on
     * read types (normal or reverse complement)
     *
     * @param [in] read_a read A pointer
     * @param [in] a_lo position on read A where the overlap starts
     * @param [in] a_hi position on read A where the overlap end
     * @param [in] a_rc true if read A is reverse complement
     * @param [in] read_b read B pointer
     * @param [in] b_lo position on read B where the overlap starts
     * @param [in] b_hi position on read B where the overlap end
     * @param [in] b_rc true if read B is reverse complement
     * @param [in] err_rate overlap error rate
     * @param [in] orig_err_rate original overlap error rate
     */
    Overlap(const Read* read_a, uint32_t a_lo, uint32_t a_hi, bool a_rc,
        const Read* read_b, uint32_t b_lo, uint32_t b_hi, bool b_rc,
        double err_rate = -1, double orig_err_rate = -1, uint32_t confirmations = 1);

    /*!
     * @brief Overlap destructor
     */
    virtual ~Overlap() {}

    /*!
     * @brief Getter for read A identifier
     * @return read A identifier
     */
    uint32_t a() const {
      return read_a_->id();
    }

    /*!
     * @brief Getter for read A
     * @return read A
     */
    const Read* read_a() const {
      return read_a_;
    }

    /*!
     * @brief Getter for hang of read A
     * @return hang of read A
     */
    int32_t a_hang() const {
        ASSERT(is_dovetail_, "Overlap", "Overlap is not dovetail!");
        return a_hang_;
    }

    /*!
     * @brief Getter for overlap start position in read A
     * @return overlap start position
     */
    uint32_t a_lo() const {
        return a_lo_;
    };

    /*!
     * @brief Getter for overlap end position in read A
     * @return overlap end position
     */
    uint32_t a_hi() const {
        return a_hi_;
    };

    /*!
     * @brief Getter for read B identifier
     * @return read B identifier
     */
    uint32_t b() const {
      return read_b_->id();
    }

    /*!
     * @brief Getter for read B
     * @return read B
     */
    const Read* read_b() const {
      return read_b_;
    }

    /*!
     * @brief Getter for hang of read A
     * @return hang of read A
     */
    int32_t b_hang() const {
        ASSERT(is_dovetail_, "Overlap", "Overlap is not dovetail!");
        return b_hang_;
    }

    /*!
     * @brief Getter for overlap start position in read B
     * @return overlap start position
     */
    uint32_t b_lo() const {
        return b_lo_;
    };

    /*!
     * @brief Getter for overlap end position in read B
     * @return overlap end position
     */
    uint32_t b_hi() const {
        return b_hi_;
    };

    /*!
     * @brief Getter for overlap length
     * @return length
     */
    uint32_t length() const;

    /*!
     * @brief Getter for overlap length in given read
     *
     * @param [in] readId read identifier
     * @return length
     */
    uint32_t length(uint32_t read_id) const;

    /*!
     * @brief Getter for amount of read covered by overlap.
     * @return covered_percentage
     */
    double covered_percentage(uint32_t read_id) const;

    /*!
     * @brief Getter for overlap type
     * @return true if overlap is innie
     */
    bool is_innie() const {
        return is_innie_;
    }

    /*!
     * @brief Getter for overlap type
     * @return true if overlap is dovetail
     */
    bool is_dovetail() const {
        return is_dovetail_;
    }

    /*!
     * @brief Getter for error rate
     * @return error rate
     */
    double err_rate() const {
        return err_rate_;
    }

    /*!
     * @breif Getter for original error rate
     * @return error rate
     */
     double orig_err_rate() const {
         return orig_err_rate_;
     }

     /*!
      * @brief Method for prefix check
      * @detals Method checks whether the start of the read is contained in overlap.
      * It respects the direction of read (important for reverse complements)!
      *
      * @param [in] read_id read identifier
      * @return true if prefix is in overlap
      */
     bool is_using_prefix(uint32_t read_id) const;

     /*!
      * @brief Method for suffix check
      * @detals Method checks whether the end of the read is contained in overlap.
      * It respects the direction of read (important for reverse complements)!
      *
      * @param [in] read_id read identifier
      * @return true if suffix is in overlap
      */
     bool is_using_suffix(uint32_t read_id) const;

     /*!
      * @brief Getter for hanging length of a read
      * @details Method returns the absolute value of the read hang.
      *
      * @param [in] read_id read identifier
      * @return hanging length
      */
     uint32_t hanging_length(uint32_t read_id) const;

     /*!
      * @brief Method for transitive overlap check
      * @details Method checks whether this (o1) is transitive considering overlaps o2 and o3.
      *
      * @param [in] o2 overlap 2
      * @param [in] o3 overlap 3
      * @return true if this is transitive
      */
     bool is_transitive(const Overlap* o2, const Overlap* o3) const;


    /*!
     * @brief Method for object cloning
     *
     * @return new Overlap object which equals this
     */
    Overlap* clone() const;

    /*!
     * @brief Method for overlap representation
     * @details Method prints overlap representation to the given stream
     *
     * @param [in] output stream
     */
    void print(std::ostream& str) const;

    /*!
     * @brief Method for extracting the overlapped part in read give by its id
     *
     * @param [in] read_id identifier of read
     * @return string containing the overlapped part
     */
    std::string extract_overlapped_part(uint32_t read_id) const;

    /*!
     * @brief Operator prints overlap representation
     * @details Operator prints given overlap representation to the given stream
     *
     * @param [in] output stream.
     * @param [in] overlap.
     */
    friend std::ostream& operator<<(std::ostream& str, Overlap const& data) {
        data.print(str);
        return str;
    }

    /*!
     * @breif Method for creating a string representation of the object
     *
     * @return string representation
     */
    std::string repr() const;

    /*!
     * @brief Method for object serialization
     * @details Serializes the object to a char array
     *
     * @param [out] bytes adress of the char array where the object is serialized
     * @param [out] bytes_length adress of the variable holding size of the
     * serialized object
     */
    void serialize(char** bytes, uint32_t* bytes_length) const;

    uint32_t confirmations() const {
      return confirmations_;
    }

    void add_confirmation() {
      confirmations_ += 1;
    }

    /*!
     * @brief Method for object deserialization
     * @details Deserializes an object stored in a char array
     *
     * @param [in] bytes char array where the object was serialized
     * @return Overlap object pointer
     */
    static Overlap* deserialize(const char* bytes);

    friend Depot;

protected:

    Overlap() {};

    static DepotObjectType type_;

    const Read* read_a_;
    int32_t a_hang_;

    const Read* read_b_;
    int32_t b_hang_;

    bool is_innie_;
    bool is_dovetail_;

    uint32_t a_lo_;
    uint32_t a_hi_;
    uint32_t b_lo_;
    uint32_t b_hi_;

    double err_rate_;
    double orig_err_rate_;

    uint32_t confirmations_;
};
