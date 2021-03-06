/*!
 * @file ReadIndex.cpp
 *
 * @brief ReadIndex class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 02, 2015
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

static int findChar(char c, const char* str, int s, int e) {

    for (int i = s; i < e; ++i) {
        if (str[i] == c) {
            return i;
        }
    }

    return -1;
}

ReadIndex::ReadIndex(const std::vector<Read*>& reads, int rk) {

    ASSERT(reads.size() > 0, "RI", "invalid number of input reads");

    Timer timer;
    timer.start();

    n_ = reads.size();

    std::string str = "";

    int f = 0;
    size_t j = 0;

    for (size_t i = 0; i < reads.size(); ++i) {

        if (str.size() + reads[i]->length() + 6 > FRAGMENT_SIZE) {

            fragmentSizes_.push_back(i - j);
            fragments_.push_back(new EnhancedSuffixArray(str));

            updateFragment(f, j, i, reads);

            str.clear();
            j = i;
            ++f;
        }

        str += S_DELIMITER;
        str += rk == 0 ? reads[i]->sequence() : reads[i]->reverse_complement();
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

    if (pattern == nullptr || m <= 0) return 0;

    size_t num = 0;

    for (size_t f = 0; f < fragments_.size(); ++f) {

        int i, j;
        findInterval(&i, &j, f, pattern, m);

        if (i == -1 && j == -1) continue;

        num += j - i + 1;
    }

    return num;
}

void ReadIndex::readDuplicates(std::vector<int>& dst, const Read* read) const {

    if (read == nullptr) return;

    std::string pattern = "";

    pattern += S_DELIMITER;
    pattern += read->sequence();
    pattern += E_DELIMITER;

    int m = pattern.size();

    for (size_t f = 0; f < fragments_.size(); ++f) {

        int i, j;
        findInterval(&i, &j, f, pattern.c_str(), m);

        if (i == -1 && j == -1) continue;

        const EnhancedSuffixArray* esa = fragments_[f];
        const std::string& str = esa->getString();

        for (int k = i; k <= j; ++k) {
            dst.push_back(*((int32_t*) (str.c_str() + esa->getSuffix(k) + m)));
        }
    }
}

void ReadIndex::readPrefixSuffixMatches(std::vector<std::pair<int, int>>& dst, const Read* read,
    int rk, int minOverlapLen) const {

    if (read == nullptr) return;

    const std::string& pattern = rk == 0 ? read->sequence() : read->reverse_complement();
    int m = pattern.size();

    int f = 0;

    for (const auto& it : fragments_) {

        int i, j, c = 0;

        const std::string& str = it->getString();

        it->intervalSubInterval(&i, &j, 1 + 5 * fragmentSizes_[f++], it->getLength() - 1, pattern[c]);

        while (i != -1 && j != -1) {

            if (i != j) {
                int l = it->intervalLcpLen(i, j);
                int del = findChar(E_DELIMITER, str.c_str(), it->getSuffix(i) + c + 1, it->getSuffix(i) + l);

                if (del == -1) {
                    int min = l < m ? l : m;

                    bool found = equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + min - 1,
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
                    del -= it->getSuffix(i); // len to delimeter
                    if (del > m) break;

                    bool found = equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + del - 1,
                        pattern.c_str(), c, del - 1);

                    if (found) {
                        for (int o = i; o <= j; ++o) {
                            dst.emplace_back(*((int32_t*) (str.c_str() + it->getSuffix(o) + del + 1)), del);
                        }
                    }

                    break;
                }

            } else {
                int del = findChar(E_DELIMITER, str.c_str(), it->getSuffix(i) + c + 1, it->getLength());
                if (del == -1) break;

                del -= it->getSuffix(i); // len to delimeter
                if (del > m) break;

                bool found = equalSubstr(str.c_str(), it->getSuffix(i) + c, it->getSuffix(i) + del - 1,
                    pattern.c_str(), c, del - 1);

                if (found) {
                    dst.emplace_back(*((int32_t*) (str.c_str() + it->getSuffix(i) + del + 1)), del);
                }

                break;
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

    if (!fileExists(path)) return nullptr;

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

void ReadIndex::findInterval(int* s, int* e, int fragment, const char* pattern, int m) const {

    *s = -1;
    *e = -1;

    if (pattern == nullptr || m <= 0) return;

    const auto& esa = fragments_[fragment];

    int i, j, c = 0;
    bool found = false;

    const std::string& str = esa->getString();
    int start = 1 + 5 * fragmentSizes_[fragment];

    esa->intervalSubInterval(&i, &j, start, esa->getLength() - 1, pattern[c]);

    while (i != -1 && j != -1 && c < m) {
        found = true;

        if (i != j) {
            int l = esa->intervalLcpLen(i, j);
            int min = l < m ? l : m;

            found = equalSubstr(str.c_str(), esa->getSuffix(i) + c, esa->getSuffix(i) + min - 1,
                pattern, c, min - 1);

            c = min;
            if (c == m) break;

            esa->intervalSubInterval(&i, &j, i, j, pattern[c]);

        } else {
            found = esa->getSuffix(i) + m > (int) str.size() ? false :
                equalSubstr(str.c_str(), esa->getSuffix(i) + c, esa->getSuffix(i) + m - 1,
                pattern, c, m - 1);

            c = m;
        }

        if (!found) break;
    }

    if (found) {
        *s = i;
        *e = j;
    }
}

void ReadIndex::updateFragment(int fragment, int start, int end, const std::vector<Read*>& reads) {

    int len = 0;
    for (int i = start; i < end; ++i) {
        len += reads[i]->length() + 2;

        *((int32_t*) (fragments_[fragment]->getString().c_str() + len)) = i;

        len += 4;
    }
}
