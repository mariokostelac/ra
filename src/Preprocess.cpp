/*
* Preprocess.cpp
*
* Created on: Apr 27, 2015
*     Author: rvaser
*/

#include "IO.hpp"
#include "ReadIndex.hpp"
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

static bool correctBase(int i, int s, std::string& sequence, int k, int c, const ReadIndex* rindex) {

    for (int j = 0; j < 3; ++j) {
        sequence[i] = switchBase(sequence[i]);

        if (rindex->getNumberOfOccurrences(&sequence[s], k) >= c) {
            return true;
        }
    }

    return false;
}

static void correctRead(Read* read, int k, int c, const ReadIndex* rindex) {

    std::string sequence(read->getSequence());

    std::vector<int> positions;
    std::vector<char> chars;

    bool correct = false;

    for (int i = 0; i < (int) sequence.size() - k + 1;) {
        if (rindex->getNumberOfOccurrences(&sequence[i], k) >= c) {
            i += k;
            continue;
        }

        if (sequence[i] == 'N') {
            correct = false;
            break;
        }

        // get left most overlapping kmer
        int s = std::max(0, i - k + 1);

        if (correctBase(i, s, sequence, k, c, rindex)) {
            i = s + k;
            correct = true;
            continue;
        }

        // get right most overlapping kmer
        s = std::min((int) sequence.size() - k, i);

        if (correctBase(i, s, sequence, k, c, rindex)) {
            i = s + k;
            correct = true;
            continue;
        }

        // correction failed
        correct = false;
        break;
    }

    if (correct) {
        for (int i = 0; i < (int) positions.size(); ++i) {
            read->correctBase(positions[i], chars[i]);
        }
    }
}

void errorCorrection(std::vector<Read*>& reads, int k, int c, const char* path) {

    Timer timer;
    timer.start();

    std::string cache(path != NULL ? path : "");
    cache += ".ra";

    ReadIndex* rindex = NULL;

    if (path != NULL && fileExists(cache.c_str())) {
        char* bytes;
        readFromFile(&bytes, cache.c_str());

        rindex = ReadIndex::deserialize(bytes);

        delete[] bytes;

    } else {
        rindex = new ReadIndex(reads);

        if (path != NULL) {
            char* bytes;
            size_t bytesLen;
            rindex->serialize(&bytes, &bytesLen);

            writeToFile(bytes, bytesLen, cache.c_str());

            delete[] bytes;
        }
    }

    for (const auto& it : reads) {
        correctRead(it, k, c, rindex);
    }

    for (const auto& it : reads) {
        std::vector<int> overlaps;
        rindex->getPrefixSuffixOverlaps(overlaps, it->getSequence().c_str(), it->getLength());
    }

    delete rindex;

    timer.stop();
    timer.print("Preproc", "error correction");
}
