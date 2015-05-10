/*
* Preprocess.cpp
*
* Created on: Apr 27, 2015
*     Author: rvaser
*/

#include "IO.hpp"
#include "ReadIndex.hpp"
#include "Preprocess.hpp"

#define MIN_KMER 15
#define MAX_KMER 45

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

static void correctReads(std::vector<Read*>& reads, int start, int end, int k, int c, const ReadIndex* rindex) {

    for (int i = start; i < end; ++i) {
        correctRead(reads[i], k, c, rindex);
    }
}

static void learnCorrectionParamC(int* c, int k, std::vector<Read*>& reads, const ReadIndex* rindex) {

    *c = -1;

    srand(time(NULL));

    std::set<int> samplesIdx;
    std::vector<Read*> samples;

    size_t samplesNum = reads.size() * 5 / 1000;

    if (samplesNum < 50) {
        samples = reads;

    } else {
        while (samplesIdx.size() != samplesNum) {

            size_t size = samplesIdx.size();
            int idx = rand() % reads.size();

            samplesIdx.insert(idx);

            if (size != samplesIdx.size()) {
                samples.push_back(reads[idx]);
            }
        }
    }

    KmerDistribution kmerDistribution;

    for (const auto& it : samples) {

        const char* sequence = it->getSequence().c_str();
        int nk = it->getLength() - k + 1;

        for (int i = 0; i < nk; ++i) {
            kmerDistribution.add(rindex->getNumberOfOccurrences(&sequence[i], k));
        }
    }

    // int threshold = kmerDistribution.getErrorBoundary();
    int threshold = kmerDistribution.getErrorBoundary(2.0f);

    if (threshold == -1) return;

    double cumulative = kmerDistribution.getCumulativeProportion(threshold);

    if (cumulative < 0.25f) {
        *c = threshold;
    }
}

static void learnCorrectionParams(int* k, int* c, std::vector<Read*>& reads, const ReadIndex* rindex) {

    *k = -1;

    for (int i = MAX_KMER; i >= MIN_KMER; --i) {

        learnCorrectionParamC(c, i, reads, rindex);

        if (*c != -1) {
            *k = i;
            break;
        }
    }
}

void errorCorrection(std::vector<Read*>& reads, int k, int c, int threadLen, const char* path) {

    threadLen = std::max(threadLen, 1);

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

    if (k == -1 && c == -1) {
        learnCorrectionParams(&k, &c, reads, rindex);
    } else if (k > 0 && c == -1) {
        learnCorrectionParamC(&c, k, reads, rindex);
    } else {
        ASSERT(k > 0, "Preproc", "invalid k-mer length k");
        ASSERT(c > 1, "Preproc", "invalid threshold c");
    }

    ASSERT(k != -1 && c != -1, "Preproc", "learning of correction parameters failed");
    fprintf(stderr, "[Preproc][error correction]: using k = %d, c = %d\n", k, c);

    int taskLen = std::ceil((double) reads.size() / threadLen);
    int start = 0;
    int end = taskLen;

    std::vector<std::thread> threads;

    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(correctReads, std::ref(reads), start, end, k, c, rindex);

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

    int max = 1000;

    if (n < 0 || n >= max) return -1;

    std::vector<int> countVector;
    toCountVector(countVector, max);

    std::int64_t sum = 0;

    for (size_t i = 0; i < countVector.size(); ++i) {
        sum += countVector[i];
    }

    // std::vector<double> cumulativeVector(countVector.size());
    std::int64_t runningSum = 0;

    // for (size_t i = 0; i < cumulativeVector.size(); ++i) {
    for (int i = 1; i <= n; ++i) {

        runningSum += countVector[i];
        //cumulativeVector[i] = (double) runningSum / sum;
    }

    return (double) runningSum / sum;
}

int KmerDistribution::getErrorBoundary() const {

    int mode = getIgnoreMode(5);
    if (mode == -1) return -1;

    // fprintf(stderr, "Trusted mode = %d\n", mode);

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

    // fprintf(stderr, "Trusted mode = %d\n", mode);

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
