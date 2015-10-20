/*!
 * @file Read.cpp
 *
 * @brief Read class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Oct 10, 2015
 */

#include "Utils.hpp"
#include "Read.hpp"

std::vector<char> kCoder = {
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  65,  66,  67,  68,  69,
    70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  -1,  -1,  -1,  -1,  -1,  -1,  65,  66,  67,
    68,  69,  70,  71,  72,  73,  74,  75,  76,  77,
    78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
    88,  89,  90,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1
};

static void threadCreateReverseComplements(std::vector<Read*>& reads,
    uint32_t start, uint32_t end) {

    for (uint32_t i = start; i < end; ++i) {
        reads[i]->create_reverse_complement();
    }
}

DepotObjectType Read::type_ = DepotObjectType::kRead;

Read::Read(uint32_t id, const std::string& name, const std::string& sequence,
    const std::string& quality, double coverage)
        : id_(id), name_(name), sequence_(), quality_(quality),
        coverage_(coverage), reverse_complement_() {

    ASSERT(name.size() > 0 && sequence.size() > 0, "Read", "invalid data");

    sequence_.reserve(sequence.size());
    for (uint32_t i = 0; i < sequence.size(); ++i) {
        auto c = kCoder[sequence[i]];
        if (c != -1) {
            sequence_.push_back(c);
        }
    }

    ASSERT(sequence_.size() > 0, "Read", "invalid data");
}

Read* Read::clone() const {

    Read* copy = new Read(id_, name_, sequence_, quality_, coverage_);

    if (!reverse_complement_.empty()) copy->create_reverse_complement();

    return copy;
}

void Read::correct_base(uint32_t idx, char c) {
    sequence_[idx] = c;
}

void Read::create_reverse_complement() {
    reverse_complement_ = reverseComplement(sequence_);
}

void Read::serialize(char** bytes, uint32_t* bytes_length) const {

    uint32_t uint32_size = sizeof(uint32_t);

    *bytes_length =
        sizeof(type_) +
        uint32_size + // id_
        uint32_size + name_.size() +
        uint32_size + sequence_.size() +
        uint32_size + quality_.size() +
        sizeof(coverage_);

    *bytes = new char[*bytes_length]();

    uint32_t field_size;
    uint32_t ptr = 0;

    // type_
    std::memcpy(*bytes + ptr, &type_, sizeof(type_));
    ptr += sizeof(type_);

    // id_
    std::memcpy(*bytes + ptr, &id_, uint32_size);
    ptr += uint32_size;

    // name_
    field_size = name_.size();
    std::memcpy(*bytes + ptr, &field_size, uint32_size);
    ptr += uint32_size;
    std::memcpy(*bytes + ptr, name_.c_str(), field_size);
    ptr += field_size;

    // sequence_
    field_size = sequence_.size();
    std::memcpy(*bytes + ptr, &field_size, uint32_size);
    ptr += uint32_size;
    std::memcpy(*bytes + ptr, sequence_.c_str(), field_size);
    ptr += field_size;

    // quality_
    field_size = quality_.size();
    std::memcpy(*bytes + ptr, &field_size, uint32_size);
    ptr += uint32_size;
    if (field_size > 1) {
        std::memcpy(*bytes + ptr, quality_.c_str(), field_size);
        ptr += field_size;
    }

    // coverage_
    std::memcpy(*bytes + ptr, &coverage_, sizeof(coverage_));
}

Read* Read::deserialize(const char* bytes) {

    auto read = new Read();

    uint32_t uint32_size = sizeof(uint32_t);
    uint32_t field_size;
    uint32_t ptr = 0;

    // type_
    DepotObjectType type;
    std::memcpy(&type, bytes + ptr, sizeof(DepotObjectType));
    ASSERT(type == type_, "Read", "Wrong object serialized in bytes array!");
    ptr += sizeof(DepotObjectType);

    // id_
    std::memcpy(&read->id_, bytes + ptr, uint32_size);
    ptr += uint32_size;

    // name_
    std::memcpy(&field_size, bytes + ptr, uint32_size);
    ptr += uint32_size;
    read->name_ = std::string(bytes + ptr, field_size);
    ptr += field_size;

    // sequence_
    std::memcpy(&field_size, bytes + ptr, uint32_size);
    ptr += uint32_size;
    read->sequence_ = std::string(bytes + ptr, field_size);
    ptr += field_size;

    // quality_
    std::memcpy(&field_size, bytes + ptr, uint32_size);
    ptr += uint32_size;
    if (field_size > 1) {
        read->quality_ = std::string(bytes + ptr, field_size);
        ptr += field_size;
    }

    // coverage_
    std::memcpy(&read->coverage_, bytes + ptr, sizeof(read->coverage_));

    return read;
}

void createReverseComplements(std::vector<Read*>& reads, int threadLen) {

    int taskLen = std::ceil((double) reads.size() / threadLen);
    int start = 0;
    int end = taskLen;

    std::vector<std::thread> threads;

    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(threadCreateReverseComplements, std::ref(reads), start, end);

        start = end;
        end = std::min(end + taskLen, (int) reads.size());
    }

    for (auto& it : threads) {
        it.join();
    }
}
