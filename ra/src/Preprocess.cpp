/*!
 * @file Preprocess.cpp
 *
 * @brief Preprocess source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Apr 27, 2015
 */


#include "IO.hpp"
#include "ReadIndex.hpp"
#include "Preprocess.hpp"

#define MIN_KMER 10
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

static bool correctBase(int i, int suffix, std::string& sequence, int k, int c, const ReadIndex* rindex) {

    for (int j = 0; j < 3; ++j) {
        sequence[i] = switchBase(sequence[i]);

        if (rindex->numberOfOccurrences(&sequence[suffix], k) >= (size_t) c) {
            return true;
        }

        if (sequence[i] == 'N') break;
    }

    return false;
}

static bool correctRead(Read* read, int k, int c, const ReadIndex* rindex) {

    std::string sequence(read->getSequence());

    std::vector<int> positions;
    std::vector<char> chars;

    bool correct = false;

    for (int i = 0; i < (int) sequence.size() - k + 1;) {
        if (rindex->numberOfOccurrences(&sequence[i], k) >= (size_t) c) {
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

    return correct;
}

static void threadCorrectReads(std::vector<Read*>& reads, int k, int c, const ReadIndex* rindex,
    int start, int end, int* totalCorrected) {

    int total = 0;

    for (int i = start; i < end; ++i) {
        if (correctRead(reads[i], k, c, rindex)) {
            ++total;
        }
    }

    *totalCorrected = total;
}

static void learnCutoff(int* c, int k, const std::vector<Read*>& reads, const ReadIndex* rindex) {

    *c = -1;

    srand(time(nullptr));

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
            kmerDistribution.add(rindex->numberOfOccurrences(&sequence[i], k));
        }
    }

    int threshold = kmerDistribution.errorBoundary(2.0f);

    if (threshold == -1) return;

    double cumulative = kmerDistribution.cumulativeProportion(threshold);

    if (cumulative < 0.25f) {
        *c = threshold;
    }
}

static void threadLearnCorrectionParams(std::vector<int>& cutoffs, const std::vector<Read*>& reads,
    const ReadIndex* rindex, int start, int end) {

    for (int k = start; k > end; --k) {

        int c;
        learnCutoff(&c, k, reads, rindex);

        if (c != -1) {
            cutoffs[k] = c;
            break;
        }
    }
}

static void learnCorrectionParams(int* k, int* c, const std::vector<Read*>& reads, const ReadIndex* rindex,
    int threadLen) {

    *k = -1;

    std::vector<int> cutoffs(MAX_KMER + 1, -1);

    int taskLen = (MAX_KMER - MIN_KMER + 1) / threadLen;
    int start = MAX_KMER;
    int end = start - taskLen;

    std::vector<std::thread> threads;

    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(threadLearnCorrectionParams, std::ref(cutoffs), std::ref(reads), rindex,
            start, end);

        start = end;
        end = (i == threadLen - 2) ? MIN_KMER - 1 : end - taskLen;
    }

    for (auto& it : threads) {
        it.join();
    }

    for (int i = MAX_KMER; i >= MIN_KMER; --i) {

        if (cutoffs[i] != -1) {
            *k = i;
            *c = cutoffs[i];
            break;
        }
    }
}

double KmerDistribution::cumulativeProportion(int n) const {

    int max = 1000;

    if (n < 0 || n >= max) return -1;

    std::vector<int> countVector;
    toCountVector(countVector, max);

    int64_t sum = 0;

    for (size_t i = 0; i < countVector.size(); ++i) {
        sum += countVector[i];
    }

    int64_t runningSum = 0;

    for (int i = 1; i <= n; ++i) {

        runningSum += countVector[i];
    }

    return (double) runningSum / sum;
}

int KmerDistribution::errorBoundary(double ratio) const {

    int mode = ignoreMode(5);
    if (mode == -1) return -1;

    std::vector<int> countVector;
    toCountVector(countVector, 1000);

    if (countVector.empty()) return -1;

    for (int i = 1; i < mode - 1; ++i) {

        int currCount = countVector[i];
        int nextCount = countVector[i + 1];

        double countRatio = nextCount == 0 ? std::numeric_limits<double>::max() : (double) currCount / nextCount;

        if (countRatio < ratio) return i;
    }

    return -1;
}

int KmerDistribution::ignoreMode(size_t n) const {

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

bool correctReads(std::vector<Read*>& reads, int k, int c, int threadLen, const char* path) {

    Timer timer;
    timer.start();

    std::string cache = path;
    cache += ".cra";

    ReadIndex* rindex = ReadIndex::load(cache.c_str());

    if (rindex == nullptr) {
        rindex = new ReadIndex(reads, 0);
        rindex->store(cache.c_str());
    }

    if (k == -1 && c == -1) {
        learnCorrectionParams(&k, &c, reads, rindex, threadLen);
    } else if (k > 0 && c == -1) {
        learnCutoff(&c, k, reads, rindex);
    } else {
        ASSERT(k > 1, "Preproc", "invalid k-mer length k");
        ASSERT(c > 1, "Preproc", "invalid threshold c");
    }

    if (k < 2 || c < 2) {
        fprintf(stderr, "[Preproc]: learning of correction parameters failed\n");

        delete rindex;
        return false;
    }

    fprintf(stderr, "[Preproc][error correction]: using k = %d, c = %d\n", k, c);

    int taskLen = std::ceil((double) reads.size() / threadLen);
    int start = 0;
    int end = taskLen;

    std::vector<std::thread> threads;
    std::vector<int> readsCorrected(threadLen);

    for (int i = 0; i < threadLen; ++i) {
        threads.emplace_back(threadCorrectReads, std::ref(reads), k, c, rindex, start, end,
            &readsCorrected[i]);

        start = end;
        end = std::min(end + taskLen, (int) reads.size());
    }

    for (auto& it : threads) {
        it.join();
    }

    int readsCorrectedTotal = 0;
    for (const auto& it : readsCorrected) readsCorrectedTotal += it;

    fprintf(stderr, "[Preproc][error correction]: correction percentage = %.2lf%%\n",
        (readsCorrectedTotal / (double) reads.size()) * 100);

    delete rindex;

    timer.stop();
    timer.print("Preproc", "error correction");

    return readsCorrectedTotal == 0 ? false : true;
}

bool filterReads(std::vector<Read*>& dst, std::vector<Read*>& reads, bool view) {

    Timer timer;
    timer.start();

    ReadIndex* rindex = new ReadIndex(reads);

    std::vector<bool> duplicates(reads.size(), false);
    std::vector<int> equals;

    for (size_t i = 0; i < reads.size(); ++i) {

        if (duplicates[i] == true) continue;

        rindex->readDuplicates(equals, reads[i]);

        reads[i]->addCoverage(equals.size() - 1);

        for (size_t j = 0; j < equals.size(); ++j) {
            duplicates[equals[j]] = true;
        }

        equals.clear();

        dst.push_back(view ? reads[i] : reads[i]->clone());
    }

    delete rindex;

    fprintf(stderr, "[Preproc][filtering]: reduction percentage = %.2lf%%\n",
        (1 - (dst.size() / (double) reads.size())) * 100);

    timer.stop();
    timer.print("Preproc", "filtering");

    return dst.size() == reads.size() ? false : true;
}
