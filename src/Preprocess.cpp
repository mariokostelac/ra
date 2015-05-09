/*
* Preprocess.cpp
*
* Created on: Apr 27, 2015
*     Author: rvaser
*/

#include "IO.hpp"
#include "ReadIndex.hpp"
#include "Preprocess.hpp"

#define MIN_KMER 20
#define MAX_KMER 50

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
            b = 'N';
            break;
    }

    return b;
}

static bool correctBase(int i, int s, std::string& sequence, int k, int c, const ReadIndex* rindex) {

    for (int j = 0; j < 3; ++j) {
        sequence[i] = switchBase(sequence[i]);

        if (rindex->getNumberOfOccurrences(&sequence[s], k) >= (size_t) c) {
            return true;
        }

        if (sequence[i] == 'N') break;
    }

    return false;
}

static void correctRead(Read* read, int k, int c, const ReadIndex* rindex) {

    std::string sequence(read->getSequence());

    std::vector<int> positions;
    std::vector<char> chars;

    bool correct = false;

    for (int i = 0; i < (int) sequence.size() - k + 1;) {
        if (rindex->getNumberOfOccurrences(&sequence[i], k) >= (size_t) c) {
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

static void correctReads(std::vector<Read*>* reads, int start, int end, int k, int c, const ReadIndex* rindex) {

    for (int i = start; i < end; ++i) {
        correctRead((*reads)[i], k, c, rindex);
    }
}

static void learnErrorCorrectionParameters(int* k, int* c, std::vector<Read*>& reads, const ReadIndex* rindex) {

    srand(time(NULL));

    std::set<int> samples;
    size_t samples_num = reads.size() * 7 / 100;

    while (samples.size() != samples_num) {
        samples.insert(rand() % reads.size());
    }

    KmerDistribution kmerDistribution;

    for (int i = MAX_KMER; i >= MIN_KMER; --i) {
        for (const auto& it : samples) {

            const char* read = reads[it]->getSequence().c_str();
            int nk = reads[it]->getLength() - i + 1;

            for (int j = 0; j < nk; ++j) {
                kmerDistribution.add(rindex->getNumberOfOccurrences(&read[j], i));
            }
        }

        int threshold = kmerDistribution.getErrorBoundary();
        int threshold2 = kmerDistribution.getErrorBoundary(2.0f);

        double cumulative = kmerDistribution.getCumulativeProportion(threshold);
        double cumulative2 = kmerDistribution.getCumulativeProportion(threshold2);

        printf("k = %d, c1 = %d c2 = %d\n", i, threshold, threshold2);
        printf("cu1 = %lf cu2 = %lf\n", cumulative, cumulative2);
    }
}

void errorCorrection(std::vector<Read*>& reads, int k, int c, int threadLen, const char* path) {

    ASSERT(threadLen > 0, "Preproc", "invalid number of threads");

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

    learnErrorCorrectionParameters(&k, &c, reads, rindex);

    std::vector<std::thread> threads;
    int taskLen = reads.size() / threadLen;
    int start = 0;
    int end = taskLen;

    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(correctReads, &reads, start, end, k, c, rindex);
        start = end;
        end = std::min(end + taskLen, (int) reads.size());
    }

    for (auto& it : threads) {
        it.join();
    }

    delete rindex;

    timer.stop();
    timer.print("Preproc", "error correction");
}

double KmerDistribution::getCumulativeProportion(int n) const {

    std::vector<int> countVector;
    toCountVector(countVector, 1000);

    std::int64_t sum = 0;

    for (size_t i = 0; i < countVector.size(); ++i) {
        sum += countVector[i];
    }

    std::vector<double> cumulativeVector(countVector.size());
    std::int64_t runningSum = 0;

    for (size_t i = 0; i < cumulativeVector.size(); ++i) {

        runningSum += countVector[i];
        cumulativeVector[i] = (double) runningSum / sum;
    }

    ASSERT(n < (int) cumulativeVector.size(), "KmerDist", "invalid n");
    return cumulativeVector[n];
}

int KmerDistribution::getErrorBoundary() const {

    int mode = getIgnoreMode(5);
    if (mode == -1) return -1;

    fprintf(stderr, "Trusted mode = %d\n", mode);

    std::vector<int> countVector;
    toCountVector(countVector, 1000);

    if (countVector.empty()) return -1;

    int sum = 0, idx = -1;
    double minContribution = std::numeric_limits<double>::max();

    for (int i = 1; i < mode; ++i) {

        sum += countVector[i];
        double contribution = (double) countVector[i] / sum;

        if (contribution < minContribution) {
            minContribution = contribution;
            idx = i;
        }
    }

    return idx;
}

int KmerDistribution::getErrorBoundary(double ratio) const {

    int mode = getIgnoreMode(5);
    if (mode == -1) return -1;

    fprintf(stderr, "Trusted mode = %d\n", mode);

    std::vector<int> countVector;
    toCountVector(countVector, 1000);

    if (countVector.empty()) return -1;

    for (int i = 1; i < mode - 1; ++i) {

        int currCount = countVector[i];
        int nextCount = countVector[i + 1];

        double countRatio = (double) currCount / nextCount;

        if (countRatio < ratio) return i;
    }

    return -1;
}

int KmerDistribution::getIgnoreMode(size_t n) const {

    std::vector<int> countVector;
    toCountVector(countVector, 1000);

    if (countVector.size() < n) return -1;

    int modeIdx = -1;
    int modeCount = -1;

    for (int i = n; i < (int) countVector.size(); ++i) {

        if (countVector[i] > modeCount) {
            modeCount = countVector[i];
            modeIdx = i;
        }
    }

    return modeIdx;
}

void KmerDistribution::toCountVector(std::vector<int>& dst, int max) const {

    if (histogram_.empty()) return;

    for (int i = 0; i < max; ++i) {
        auto it = histogram_.find(i);
        dst.push_back(it != histogram_.end() ? it->second : 0);
    }
}
