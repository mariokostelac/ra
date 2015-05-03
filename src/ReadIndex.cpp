/*
* ReadIndex.cpp
*
* Created on: May 02, 2015
*     Author: rvaser
*/

#include "ReadIndex.hpp"

#define FRAGMENT_SIZE 2000000000 // ~2GB

ReadIndex::ReadIndex(const Read* read, int rk) {

    offsets_.push_back(0);

    dictionary_.resize(1);
    dictionary_[0].push_back(read->getLength());

    fragments_.push_back(new EnhancedSuffixArray(rk == 0 ? read->getSequence() : read->getReverseComplement()));
}

ReadIndex::ReadIndex(const std::vector<Read*>& reads, int rk) {

    Timer timer;
    timer.start();

    offsets_.push_back(0);
    dictionary_.resize(1);

    size_t length = 0;

    std::vector<const std::string*> vstr;

    for (size_t i = 0; i < reads.size(); ++i) {

        if (length + reads[i]->getLength() > FRAGMENT_SIZE) {

            offsets_.push_back(i);
            dictionary_.resize(dictionary_.size() + 1);

            fragments_.push_back(new EnhancedSuffixArray(vstr));

            vstr.clear();
            length = 0;
        }

        vstr.push_back(&(rk == 0 ? reads[i]->getSequence() : reads[i]->getReverseComplement()));

        length += reads[i]->getLength() + 1;
        dictionary_.back().push_back(length);
    }

    fragments_.push_back(new EnhancedSuffixArray(vstr));

    timer.stop();
    timer.print("RI", "construction");
}

ReadIndex::~ReadIndex() {

    for (const auto& it : fragments_) {
        delete it;
    }
}

int ReadIndex::getNumberOfOccurrences(const char* pattern, int m) const {

    int num = 0;
    for (const auto& it : fragments_) {
        num += it->getNumberOfOccurrences(pattern, m);
    }

    return num;
}

void ReadIndex::serialize(char** bytes, size_t* bytesLen) const {

    *bytesLen = getSizeInBytes();
    *bytes = new char[*bytesLen];

    size_t size = sizeof(int);
    size_t ptr = 0;

    int numFragments = fragments_.size();

    std::memcpy(*bytes, &numFragments, size);
    ptr += size;

    std::memcpy(*bytes + ptr, &offsets_[0], numFragments * size);
    ptr += numFragments * size;

    for (const auto& it : dictionary_) {

        int numReads = it.size();

        std::memcpy(*bytes + ptr, &numReads, size);
        ptr += size;

        std::memcpy(*bytes + ptr, &it[0], numReads * size);
        ptr += numReads * size;
    }

    for (const auto& it : fragments_) {

        char* bytesPart;
        size_t bytesPartLen;
        it->serialize(&bytesPart, &bytesPartLen);

        std::memcpy(*bytes + ptr, &bytesPartLen, sizeof(size_t));
        ptr += sizeof(size_t);

        std::memcpy(*bytes + ptr, &bytesPart[0], bytesPartLen);
        ptr += bytesPartLen;

        delete[] bytesPart;
    }
}

ReadIndex* ReadIndex::deserialize(char* bytes) {

    Timer timer;
    timer.start();

    ReadIndex* rindex = new ReadIndex();

    size_t size = sizeof(int);
    size_t ptr = 0;

    int numFragments = 0;

    std::memcpy(&numFragments, bytes, size);
    ptr += size;

    rindex->offsets_.resize(numFragments);
    rindex->dictionary_.resize(numFragments);

    std::memcpy(&rindex->offsets_[0], bytes + ptr, numFragments * size);
    ptr += numFragments * size;

    for (int i = 0; i < numFragments; ++i) {

        int numReads = 0;

        std::memcpy(&numReads, bytes + ptr, size);
        ptr += size;

        rindex->dictionary_[i].resize(numReads);

        std::memcpy(&rindex->dictionary_[i][0], bytes + ptr, numReads * size);
        ptr += numReads * size;
    }

    for (int i = 0; i < numFragments; ++i) {

        size_t bytesPartLen = 0;

        std::memcpy(&bytesPartLen, bytes + ptr, sizeof(size_t));
        ptr += sizeof(size_t);

        rindex->fragments_.push_back(EnhancedSuffixArray::deserialize(bytes + ptr));
        ptr += bytesPartLen;
    }

    timer.stop();
    timer.print("RI", "cached construction");

    return rindex;
}

size_t ReadIndex::getSizeInBytes() const {
    
    size_t bytesLen = 0;
    size_t size = sizeof(int);

    bytesLen += size; // number of fragments
    bytesLen += offsets_.size() * size;

    for (const auto& it : dictionary_) {
        bytesLen += size;
        bytesLen += it.size() * size;
    }

    for (const auto& it : fragments_) {
        bytesLen += sizeof(size_t);
        bytesLen += it->getSizeInBytes();
    }

    return bytesLen;
}

int ReadIndex::getIndex(int fragment, int position) {

    auto up = std::upper_bound(dictionary_[fragment].begin(), dictionary_[fragment].end(), position);
    return up - dictionary_[fragment].begin();
}
