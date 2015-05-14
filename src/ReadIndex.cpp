/*
* ReadIndex.cpp
*
* Created on: May 02, 2015
*     Author: rvaser
*/

#include "IO.hpp"
#include "ReadIndex.hpp"

#define FRAGMENT_SIZE 2147483645U // 2GB - 2 for sentinels

ReadIndex::ReadIndex(const Read* read, int rk) {

    n_ = 1;

    offsets_.push_back(0);

    dictionary_.resize(1);
    dictionary_[0].push_back(read->getLength());

    fragments_.push_back(new EnhancedSuffixArray(rk == 0 ? read->getSequence() : read->getReverseComplement()));
}

ReadIndex::ReadIndex(const std::vector<Read*>& reads, int rk) {

    Timer timer;
    timer.start();

    n_ = reads.size();

    offsets_.push_back(0);
    dictionary_.resize(1);

    size_t length = 0;

    std::vector<const std::string*> vstr;

    for (size_t i = 0; i < reads.size(); ++i) {

        if (length + reads[i]->getLength() + 1 > FRAGMENT_SIZE) {

            offsets_.push_back(i);
            dictionary_.resize(dictionary_.size() + 1);

            fragments_.push_back(new EnhancedSuffixArray(vstr));

            vstr.clear();
            length = 0;
        }

        vstr.push_back(&(rk == 0 ? reads[i]->getSequence() : reads[i]->getReverseComplement()));

        length += reads[i]->getLength() + 1; // DELIMITER
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

size_t ReadIndex::getNumberOfOccurrences(const char* pattern, int m) const {

    if (pattern == NULL || m <= 0) return 0;

    size_t num = 0;
    for (const auto& it : fragments_) {

        int i, j;
        it->getInterval(&i, &j, pattern, m);

        num += (i == -1 && j == -1) ? 0 : j - i + 1;
    }

    return num;
}

void ReadIndex::getPrefixSuffixOverlaps(std::vector<int>& dst, const char* pattern, int m, int minOverlapLen) const {

    if (pattern == NULL || m <= 0) return;

    dst.clear();
    dst.resize(n_, 0);

    for (int f = 0; f < (int) fragments_.size(); ++f) {

        int i = 0, j = fragments_[f]->getLength() - 1;

        for (int c = 0; c < m; ++c) {
            fragments_[f]->getSubInterval(&i, &j, i, j, pattern[c]);

            if (i == -1 && j == -1) break;
            if (c < minOverlapLen - 1) continue;

            int k, l;
            fragments_[f]->getSubInterval(&k, &l, i, j, DELIMITER);

            if (k == -1 && l == -1) continue;

            for (int o = k; o < l + 1; ++o) {
                dst[getIndex(f, fragments_[f]->getSuffix(o))] = c + 1;
            }
        }
    }
}

void ReadIndex::serialize(char** bytes, size_t* bytesLen) const {

    *bytesLen = getSizeInBytes();
    *bytes = new char[*bytesLen];

    size_t size = sizeof(int);
    size_t ptr = 0;

    std::memcpy(*bytes + ptr, &n_, size);
    ptr += size;

    int numFragments = fragments_.size();

    std::memcpy(*bytes + ptr, &numFragments, size);
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

    std::memcpy(&rindex->n_, bytes + ptr, size);
    ptr += size;

    int numFragments = 0;

    std::memcpy(&numFragments, bytes + ptr, size);
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

    bytesLen += size; // n_
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

int ReadIndex::getIndex(int fragment, int position) const {

    auto up = std::upper_bound(dictionary_[fragment].begin(), dictionary_[fragment].end(), position);
    return up - dictionary_[fragment].begin();
}

ReadIndex* createReadIndex(const std::vector<Read*>& reads, int rk, const char* path, const char* ext) {

    std::string cache(path != NULL ? path : "");
    cache += ".";
    cache += ext;

    ReadIndex* rindex = NULL;

    if (path != NULL && fileExists(cache.c_str())) {
        char* bytes;
        readFromFile(&bytes, cache.c_str());

        rindex = ReadIndex::deserialize(bytes);

        delete[] bytes;

    } else {
        rindex = new ReadIndex(reads, rk);

        if (path != NULL) {
            char* bytes;
            size_t bytesLen;
            rindex->serialize(&bytes, &bytesLen);

            writeToFile(bytes, bytesLen, cache.c_str());

            delete[] bytes;
        }
    }

    return rindex;
}
