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

    uint32_t size = sizeof(uint32_t);

    *bytes_length =
        size + // id_
        size + name_.size() + 1 +
        size + sequence_.size() + 1 +
        size + quality_.size() + 1 +
        sizeof(coverage_);

    *bytes = new char[*bytes_length]();

    uint32_t temp;
    uint32_t ptr = 0;

    // id_
    std::memcpy(*bytes + ptr, &id_, size);
    ptr += size;

    // name_
    temp = name_.size() + 1;
    std::memcpy(*bytes + ptr, &temp, size);
    ptr += size;
    std::memcpy(*bytes + ptr, &name_[0], temp);
    ptr += temp;

    // sequence_
    temp = sequence_.size() + 1;
    std::memcpy(*bytes + ptr, &temp, size);
    ptr += size;
    std::memcpy(*bytes + ptr, &sequence_[0], temp);
    ptr += temp;

    // quality_
    temp = quality_.size() + 1;
    std::memcpy(*bytes + ptr, &temp, size);
    ptr += size;
    if (temp > 1) {
        std::memcpy(*bytes + ptr, &quality_[0], temp);
        ptr += quality_.size();
    }

    // coverage_
    std::memcpy(*bytes + ptr, &coverage_, sizeof(coverage_));
}

AfgRead* AfgRead::deserialize(const char* bytes) {

    AfgRead* read = new AfgRead();

    uint32_t size = sizeof(uint32_t);
    uint32_t temp;
    uint32_t ptr = 0;

    // id_
    std::memcpy(&read->id_, bytes + ptr, size);
    ptr += size;

    // name_
    std::memcpy(&temp, bytes + ptr, size);
    ptr += size;
    read->name_.reserve(temp);
    std::memcpy(&read->name_[0], bytes + ptr, temp);
    ptr += temp;

    // sequence_
    std::memcpy(&temp, bytes + ptr, size);
    ptr += size;
    read->sequence_.reserve(temp);
    std::memcpy(&read->sequence_[0], bytes + ptr, temp);
    ptr += temp;

    // quality_
    std::memcpy(&temp, bytes + ptr, size);
    ptr += size;
    if (temp > 1) {
        read->quality_.reserve(temp);
        std::memcpy(&read->quality_[0], bytes + ptr, temp);
        ptr += temp;
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
