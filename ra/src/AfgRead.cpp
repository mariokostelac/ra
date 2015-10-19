/*!
 * @file Read.cpp
 *
 * @brief Read class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Apr 20, 2015
 */

#include "AfgRead.hpp"

static bool isValidChar(char c) {
    if ((c > 64 && c < 91) || (c > 96 && c < 123)) return true;
    return false;
}

static void threadCreateReverseComplements(std::vector<Read*>& reads, int start, int end) {

    for (int i = start; i < end; ++i) {
        reads[i]->createReverseComplement();
    }
}

AfgRead::AfgRead(int id, const std::string& name, const std::string& sequence, const std::string& quality, double coverage) {

    ASSERT(name.size() > 0 && sequence.size() > 0, "Read", "invalid data");

    id_ = id;
    name_ = name;

    for (int i = 0; i < (int) sequence.size(); ++i) {
        if (isValidChar(sequence[i])) {
            sequence_ += toupper(sequence[i]);
        }
    }

    ASSERT(sequence_.size() > 0, "Read", "invalid data");

    quality_ = quality;
    coverage_ = coverage;
    reverseComplement_ = "";
}

AfgRead* AfgRead::clone() const {

    AfgRead* copy = new AfgRead(id_, name_, sequence_, quality_, coverage_);

    if (!reverseComplement_.empty()) copy->createReverseComplement();

    return copy;
}

void AfgRead::correctBase(int idx, int c) {
    sequence_[idx] = c;
}

void AfgRead::createReverseComplement() {

    reverseComplement_ = reverse_complement(sequence_);
}

void AfgRead::serialize(char** bytes, uint32_t* bytes_length) const {

    uint32_t uint32_size = sizeof(uint32_t);

    *bytes_length =
        uint32_size + // id_
        uint32_size + name_.size() +
        uint32_size + sequence_.size() +
        uint32_size + quality_.size() +
        sizeof(coverage_);

    *bytes = new char[*bytes_length]();

    uint32_t field_size;
    uint32_t ptr = 0;

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

AfgRead* AfgRead::deserialize(const char* bytes) {

    AfgRead* read = new AfgRead();

    uint32_t uint32_size = sizeof(uint32_t);
    uint32_t field_size;
    uint32_t ptr = 0;

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
