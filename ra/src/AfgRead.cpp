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
