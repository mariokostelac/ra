/*
* ReadIndex.cpp
*
* Created on: May 02, 2015
*     Author: rvaser
*/

#include "ReadIndex.hpp"

#define S_DELIMITER '%'
#define E_DELIMITER '#'
#define SUBSTITUTE "$$$$"

#define FRAGMENT_SIZE 2147483645U // 2GB - 2B for sentinels

static bool equalSubstr(const char* str1, int s1, int e1, const char* str2, int s2, int e2) {

    if (e1 - s1 != e2 - s2) return false;

    for (int i = 0; i <= e1 - s1; ++i) {
        if (str1[s1 + i] != str2[s2 + i]) return false;
    }

    return true;
}

ReadIndex::ReadIndex(const std::vector<ReadPtr>& reads, int rk) {

    ASSERT(reads.size() > 0, "RI", "invalid number of input reads");

    Timer timer;
    timer.start();

    n_ = reads.size();

    std::string str = "";

    int f = 0;
    size_t j = 0;

    for (size_t i = 0; i < reads.size(); ++i) {

        if (str.size() + reads[i]->getLength() + 6 > FRAGMENT_SIZE) {

            fragmentSizes_.push_back(i - j);
            fragments_.push_back(new EnhancedSuffixArray(str));

            updateFragment(f, j, i, reads);

            str.clear();
            j = i;
            ++f;
        }

        str += S_DELIMITER;
        str += rk == 0 ? reads[i]->getSequence() : reads[i]->getReverseComplement();
        str += E_DELIMITER;
        str += SUBSTITUTE;
    }

    fragmentSizes_.push_back(reads.size() - j);
    fragments_.push_back(new EnhancedSuffixArray(str));

    updateFragment(f, j, reads.size(), reads);

    timer.stop();
    timer.print("RI", "construction");
}

ReadIndex::~ReadIndex() {

    for (const auto& it : fragments_) {
        delete it;
    }
}

size_t ReadIndex::numberOfOccurrences(const char* pattern, int m) const {

    if (pattern == NULL || m <= 0) return 0;

    size_t num = 0;
    int f = 0;

    for (const auto& it : fragments_) {

        int i, j, c = 0;
        bool found = false;

        const std::string& str = it->getString();
        int start = 1 + 6 * fragmentSizes_[f++];

        it->intervalSubInterval(&i, &j, start, it->getLength() - 1, pattern[c]);

        while (i != -1 && j != -1 && c < m) {
            found = true;

            if (i != j) {
                int l = it->intervalLcpLen(i, j);
                int min = l < m ? l : m;

                found = equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + min - 1,
                    pattern, c, min - 1);

                c = min;
                if (c == m) break;

                it->intervalSubInterval(&i, &j, i, j, pattern[c]);

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

void ReadIndex::readDuplicates(std::vector<int>& dst, const Read* read) const {

    if (read == NULL) return;

    std::string pattern = "";
    pattern += S_DELIMITER;
    pattern += read->getSequence();
    pattern += E_DELIMITER;

    int m = pattern.size();

    int f = 0;

    for (const auto& it : fragments_) {

        int i, j, c = 0;
        bool found = false;

        const std::string& str = it->getString();

        it->intervalSubInterval(&i, &j, 1 + 5 * fragmentSizes_[f++], it->getLength() - 1, pattern[c]);

        while (i != -1 && j != -1 && c < m) {
            found = true;

            if (i != j) {
                int l = it->intervalLcpLen(i, j);
                int min = l < m ? l : m;

                found = equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + min - 1,
                    pattern.c_str(), c, min - 1);

                c = min;
                if (c == m) break;

                it->intervalSubInterval(&i, &j, i, j, pattern[c]);

            } else {
                found = it->getSuffix(i) + m > (int) str.size() ? false :
                    equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + m - 1,
                    pattern.c_str(), c, m - 1);

                c = m;
            }

            if (!found) break;
        }

        for (int e = i; e <= j; ++e) {
            dst.push_back(*((int32_t*) (str.c_str() + it->getSuffix(e) + m)));
        }
    }
}

void ReadIndex::readPrefixSuffixMatches(std::vector<std::pair<int, int>>& dst, const Read* read,
    int rk, int minOverlapLen) const {

    if (read == NULL) return;

    const std::string& pattern = rk == 0 ? read->getSequence() : read->getReverseComplement();
    int m = pattern.size();

    int f = 0;

    for (const auto& it : fragments_) {

        int i, j, c = 0;
        bool found = false;

        const std::string& str = it->getString();

        it->intervalSubInterval(&i, &j, 1 + 6 * fragmentSizes_[f++], it->getLength() - 1, pattern[c]);

        while (i != -1 && j != -1 && c < m) {

            if (i != j) {
                int l = it->intervalLcpLen(i, j);
                int min = l < m ? l : m;

                found = equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + min - 1,
                    pattern.c_str(), c, min - 1);

                if (!found) break;
                c = min;

                if (c == m) {
                    for (int o = i ; o <= j; ++o) {
                        if (it->getSuffix(o) + m < it->getLength() && str[it->getSuffix(o) + m] == E_DELIMITER) {
                            dst.emplace_back(*((int32_t*) (str.c_str() + it->getSuffix(o) + m + 1)), m);
                        }
                    }
                    break;

                } else {
                    int b, d;
                    it->intervalSubInterval(&b, &d, i, j, E_DELIMITER);

                    if (b != -1 && d != -1 && min >= minOverlapLen) {
                        for (int o = b; o <= d; ++o) {
                            dst.emplace_back(*((int32_t*) (str.c_str() + it->getSuffix(o) + min + 1)), min);
                        }
                    }
                }

                it->intervalSubInterval(&i, &j, i, j, pattern[c]);

            } else {
                if (it->getSuffix(i) + m > it->getLength() - 1) break;

                if (equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + m - 1,
                    pattern.c_str(), c, m - 1) && str[it->getSuffix(i) + m] == E_DELIMITER) {

                    dst.emplace_back(*((int32_t*) (str.c_str() + it->getSuffix(i) + m + 1)), m);
                }

                c = m;
            }
        }
    }
}

size_t ReadIndex::sizeInBytes() const {

    size_t bytesLen = 0;
    size_t size = sizeof(int);

    bytesLen += size; // n_
    bytesLen += size; // number of fragments
    bytesLen += fragmentSizes_.size() * size;

    for (const auto& it : fragments_) {
        bytesLen += sizeof(size_t);
        bytesLen += it->sizeInBytes();
    }

    return bytesLen;
}

void ReadIndex::serialize(char** bytes, size_t* bytesLen) const {

    *bytesLen = sizeInBytes();
    *bytes = new char[*bytesLen];

    size_t size = sizeof(int);
    size_t ptr = 0;

    std::memcpy(*bytes + ptr, &n_, size);
    ptr += size;

    int numFragments = fragments_.size();

    std::memcpy(*bytes + ptr, &numFragments, size);
    ptr += size;

    std::memcpy(*bytes + ptr, &fragmentSizes_[0], numFragments * size);
    ptr += numFragments * size;

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

    ReadIndex* rindex = new ReadIndex();

    size_t size = sizeof(int);
    size_t ptr = 0;

    std::memcpy(&rindex->n_, bytes + ptr, size);
    ptr += size;

    int numFragments = 0;

    std::memcpy(&numFragments, bytes + ptr, size);
    ptr += size;

    rindex->fragmentSizes_.resize(numFragments);

    std::memcpy(&rindex->fragmentSizes_[0], bytes + ptr, numFragments * size);
    ptr += numFragments * size;

    for (int i = 0; i < numFragments; ++i) {

        size_t bytesPartLen = 0;

        std::memcpy(&bytesPartLen, bytes + ptr, sizeof(size_t));
        ptr += sizeof(size_t);

        rindex->fragments_.push_back(EnhancedSuffixArray::deserialize(bytes + ptr));
        ptr += bytesPartLen;
    }

    return rindex;
}

void ReadIndex::store(const char* path) const {

    char* bytes;
    size_t bytesLen;
    serialize(&bytes, &bytesLen);

    fileWrite(bytes, bytesLen, path);

    delete[] bytes;
}

ReadIndex* ReadIndex::load(const char* path) {

    if (!fileExists(path)) return NULL;

    Timer timer;
    timer.start();

    char* bytes;
    fileRead(&bytes, path);

    ReadIndex* rindex = deserialize(bytes);

    delete[] bytes;

    timer.stop();
    timer.print("RI", "cached construction");

    return rindex;
}

void ReadIndex::updateFragment(int fragment, int start, int end, const std::vector<ReadPtr>& reads) {

    int len = 0;
    for (int i = start; i < end; ++i) {
        len += reads[i]->getLength() + 2;

        *((int32_t*) (fragments_[fragment]->getString().c_str() + len)) = i;

        len += 4;
    }
}
