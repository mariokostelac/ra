/*
* ReadIndex.cpp
*
* Created on: May 02, 2015
*     Author: rvaser
*/

#include "IO.hpp"
#include "ReadIndex.hpp"

#define DELIMITER '#'

#define FRAGMENT_SIZE 2147483645U // 2GB - 2B for sentinels

static bool equalSubstr(const char* str1, int s1, int e1, const char* str2, int s2, int e2) {

    if (e1 - s1 != e2 - s2) return false;

    for (int i = 0; i <= e1 - s1; ++i) {
        if (str1[s1 + i] != str2[s2 + i]) return false;
    }

    return true;
}

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

    std::string str;

    for (size_t i = 0; i < reads.size(); ++i) {

        if (str.size() + reads[i]->getLength() + 1 > FRAGMENT_SIZE) {

            offsets_.push_back(i);
            dictionary_.resize(dictionary_.size() + 1);

            fragments_.push_back(new EnhancedSuffixArray(str));

            str.clear();
        }

        str += rk == 0 ? reads[i]->getSequence() : reads[i]->getReverseComplement();
        str += DELIMITER;

        dictionary_.back().push_back(str.size());
    }

    fragments_.push_back(new EnhancedSuffixArray(str));

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

        int i, j, c = 0;
        bool found = false;

        const std::string& str = it->getString();

        it->getInterval(&i, &j, 0, it->getLength() - 1, pattern[c]);

        while (i != -1 && j != -1 && c < m) {
            found = true;

            if (i != j) {
                int l = it->getLcpLen(i, j);
                int min = l < m ? l : m;

                found = equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + min - 1,
                    pattern, c, min - 1);

                c = min;
                if (c == m) break;

                it->getInterval(&i, &j, i, j, pattern[c]);

            } else {
                found = it->getSuffix(i) + m > (int) str.size() ? false :
                    equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + m - 1,
                    pattern, c, m - 1);

                c = m;
            }

            if (!found) break;
        }

        num += found ? j - i + 1 : 0;
    }

    return num;
}

void ReadIndex::getPrefixSuffixMatches(std::vector<int>& dst, const char* pattern, int m, int minOverlapLen) const {

    if (pattern == NULL || m <= 0) return;
    if (m < minOverlapLen) return;

    dst.clear();
    dst.resize(n_, 0);

    int f = 0;
    for (const auto& it : fragments_) {

        int i, j, c = 0;
        bool found = false;

        const std::string& str = it->getString();

        it->getInterval(&i, &j, 0, it->getLength() - 1, pattern[c]);

        while (i != -1 && j != -1 && c < m) {

            if (i != j) {
                int l = it->getLcpLen(i, j);
                int min = l < m ? l : m;

                found = equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + min - 1,
                    pattern, c, min - 1);

                if (!found) break;
                c = min;

                if (c == m) {
                    for (int o = i ; o <= j; ++o) {
                        if (it->getSuffix(0) + m < it->getLength() && str[it->getSuffix(o) + m] == DELIMITER) {
                            dst[getIndex(f, it->getSuffix(o))] = m;
                        }
                    }
                    break;

                } else {
                    int b, d;
                    it->getInterval(&b, &d, i, j, DELIMITER);

                    if (b != -1 && d != -1 && min >= minOverlapLen) {
                        for (int o = b; o <= d; ++o) dst[getIndex(f, it->getSuffix(o))] = min;
                    }
                }

                it->getInterval(&i, &j, i, j, pattern[c]);

            } else {
                if (it->getSuffix(i) + m > it->getLength() - 1) break;

                if (equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + m - 1,
                    pattern, c, m - 1) && str[it->getSuffix(i) + m] == DELIMITER) {

                    dst[getIndex(f, it->getSuffix(i))] = m;
                }

                c = m;
            }
        }

        ++f;
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
