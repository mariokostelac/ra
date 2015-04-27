/*
* Preprocess.cpp
*
* Created on: Apr 27, 2015
*     Author: rvaser
*/

#include "IO.hpp"
#include "EnhancedSuffixArray.hpp"
#include "Preprocess.hpp"

static char switchBase(char c) {

    char b;

    switch (c) {
        case 'A':
            b = 'C';
            break;
        case 'C':
            b = 'G';
            break;
        case 'G':
            b = 'T';
            break;
        case 'T':
            b = 'A';
            break;
        default:
            printf("Unkown char %c\n", c);
            b = 'N';
            break;
    }

    return b;
}

static bool correctBase(int i, int s, std::string& sequence, int k, int c, const EnhancedSuffixArray* esa) {

    for (int j = 0; j < 3; ++j) {
        sequence[i] = switchBase(sequence[i]);

        if (esa->getNumberOfOccurrences(&sequence[s], k) >= c) {
            return true;
        }
    }

    return false;
}

static void correctRead(Read* read, int k, int c, const EnhancedSuffixArray* esa) {

    std::string sequence(read->getSequence());

    std::vector<int> positions;
    std::vector<char> chars;

    bool correct = false;

    for (int i = 0; i < (int) sequence.size() - k + 1;) {
        if (esa->getNumberOfOccurrences(&sequence[i], k) >= c) {
            i += k;
            continue;
        }

        if (sequence[i] == 'N') {
            correct = false;
            break;
        }

        // get left most overlapping kmer
        int s = std::max(0, i - k + 1);

        if (correctBase(i, s, sequence, k, c, esa)) {
            i = s + k;
            correct = true;
            continue;
        }

        // get right most overlapping kmer
        s = std::min((int) sequence.size() - k, i);

        if (correctBase(i, s, sequence, k, c, esa)) {
            i = s + k;
            correct = true;
            continue;
        }

        // correction failed
        correct = false;
        break;
    }

    if (correct) {
        printf("Correcting read\n");
        for (int i = 0; i < (int) positions.size(); ++i) {
            read->correctBase(positions[i], chars[i]);
        }
    }
}

void errorCorrection(std::vector<Read*>& reads, int k, int c, const char* path) {

    std::string cache(path);
    cache += ".ra";

    EnhancedSuffixArray* esa = NULL;

    if (path != NULL && fileExists(cache.c_str())) {
        char* bytes;
        readFromFile(&bytes, cache.c_str());

        esa = EnhancedSuffixArray::deserialize(bytes);

        delete[] bytes;

    } else {
        esa = new EnhancedSuffixArray(reads);

        if (path != NULL) {
            char* bytes;
            int bytesLen;
            esa->serialize(&bytes, &bytesLen);

            writeToFile(bytes, bytesLen, cache.c_str());

            delete[] bytes;
        }
    }

    for (const auto& it : reads) {
        correctRead(it, k, c, esa);
    }

    delete esa;
}
