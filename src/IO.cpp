/*
* IO.hpp
*
* Created on: Apr 21, 2015
*     Author: rvaser
*/

#include "IO.hpp"

#define BUFFER_SIZE 4096

static FILE* fileSafeOpen(const char* path, const char* mode) {
    FILE* f = fopen(path, mode);
    ASSERT(f != NULL, "IO", "cannot open file %s with mode %s", path, mode);
    return f;
}

const struct option Options::options_[] = {
    {"reads", required_argument, 0, 'i'},
    {"threads", required_argument, 0, 't'},
    {"kmer", required_argument, 0, 'k'},
    {"threshold", required_argument, 0, 'c'},
    {"min-overlap-length", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

Options::Options(const char* readsPath, int threadLen, int k, int c, int minOverlapLen) :
    readsPath(readsPath), threadLen(threadLen), k(k), c(c), minOverlapLen(minOverlapLen) {
}

Options* Options::parseOptions(int argc, char** argv) {

    char* readsPath = NULL;
    int threadLen = std::max(std::thread::hardware_concurrency(), 1U);
    int k = -1;
    int c = -1;
    int minOverlapLen = 5;

    while (1) {

        char argument = getopt_long(argc, argv, "i:t:o:h", options_, NULL);

        if (argument == -1) {
            break;
        }

        switch (argument) {
        case 'i':
            readsPath = optarg;
            break;
        case 't':
            threadLen = atoi(optarg);
            break;
        case 'k':
            k = atoi(optarg);
            break;
        case 'c':
            c = atoi(optarg);
            break;
        case 'o':
            minOverlapLen = atoi(optarg);
            break;
        default:
            help();
            return NULL;
        }
    }

    ASSERT(readsPath, "IO", "missing option -i (reads file)");
    ASSERT(threadLen > 0, "IO", "invalid thread number");
    ASSERT(minOverlapLen > 0, "IO", "invalid minimal overlap length");

    return new Options(readsPath, threadLen, k, c, minOverlapLen);
}

void Options::help() {

    printf(
    "usage: ra -i <read file> [arguments ...]\n"
    "\n"
    "arguments:\n"
    "    -i, --reads <file>\n"
    "        (required)\n"
    "        input fasta/fastq reads file\n"
    "    -t, --threads <int>\n"
    "        default: approx. number of processors/cores\n"
    "        number of threads used\n"
    "    --kmer <int>\n"
    "        default: based on dataset\n"
    "        length of k-mers used in error correction\n"
    "    --threshold <int>\n"
    "        default: based on dataset\n"
    "        minimal number of occurrences for a k-mer to not be erroneous\n"
    "    -o, --min-overlap-length <int>\n"
    "        default: 5\n"
    "        minimal length of exact overlap between two reads\n"
    "    -h, -help\n"
    "        prints out the help\n");
}

void readFastaReads(std::vector<Read*>& reads, const char* path) {

    Timer timer;
    timer.start();

    FILE* f = fileSafeOpen(path, "r");

    std::string name;
    std::string sequence;

    char* buffer = new char[BUFFER_SIZE];

    bool isName = false;
    bool createRead = false;

    int idx = 0;

    while (!feof(f)) {

        int readLen = fread(buffer, sizeof(char), BUFFER_SIZE, f);

        for (int i = 0; i < readLen; ++i) {

            if (buffer[i] == '>') {

                if (createRead) {
                    reads.push_back(new Read(idx++, name, sequence, "", 1.0));
                }

                name.clear();
                sequence.clear();

                createRead = true;
                isName = true;

            } else {
                switch (buffer[i]) {
                    case '\r':
                        break;
                    case '\n':
                        isName = false;
                        break;
                    default:
                        isName ? name += buffer[i] : sequence += buffer[i];
                        break;
                }
            }
        }
    }

    reads.push_back(new Read(idx, name, sequence, "", 1.0));

    delete[] buffer;
    fclose(f);

    timer.stop();
    timer.print("IO", "fasta input");
}

void readFastqReads(std::vector<Read*>& reads, const char* path) {

    Timer timer;
    timer.start();

    ASSERT(fileExists(path), "IO", "cannot open file %s with mode r", path);

    std::ifstream f(path);
    std::string line;

    std::string name;
    std::string sequence;
    std::string quality;

    int i = 0;
    int idx = 0;

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        switch (i % 4) {
            case 0:
                if (i != 0) {
                    reads.push_back(new Read(idx++, name, sequence, quality, 1.0));
                }

                name = line.substr(1, line.size() - 1);
                sequence.clear();
                quality.clear();
                break;
            case 1:
                sequence = line;
                break;
            case 3:
                quality = line;
                break;
            default:
                break;
        }

        ++i;
    }

    reads.push_back(new Read(idx, name, sequence, quality, 1.0));

    f.close();

    timer.stop();
    timer.print("IO", "fastq input");
}

void readAfgReads(std::vector<Read*>& reads, const char* path) {

    Timer timer;
    timer.start();

    ASSERT(fileExists(path), "IO", "cannot open file %s with mode r", path);

    std::ifstream f(path);
    AMOS::Reader* reader = new AMOS::Reader(f);

    while (reader->has_next()) {

        Read* read = nullptr;

        if (reader->next(&read)) {
            reads.emplace_back(read);
        }
    }

    delete reader;
    f.close();

    timer.stop();
    timer.print("IO", "afg input");
}

void writeAfgReads(const std::vector<Read*>& reads, const char* path) {

    Timer timer;
    timer.start();

    std::ofstream f(path);

    for (const auto& read : reads) {
        f << "{RED" << std::endl;
        f << "clr:0," << read->getLength() << std::endl;
        f << "eid:" << read->getName() << std::endl;
        f << "iid:" << read->getId() << std::endl;
        f << "qlt:" << read->getQuality() << std::endl;
        f << "." << std::endl;
        f << "seq:" << read->getSequence() << std::endl;
        f << "." << std::endl;
        f << "cvg:" << read->getCoverage() << std::endl;
        f << "}" << std::endl;
    }

    f.close();

    timer.stop();
    timer.print("IO", "afg output");
}

void readAfgOverlaps(std::vector<Overlap*>& overlaps, const char* path) {

    Timer timer;
    timer.start();

    ASSERT(fileExists(path), "IO", "cannot open file %s with mode r", path);

    std::ifstream f(path);
    AMOS::Reader* reader = new AMOS::Reader(f);

    while (reader->has_next()) {

        Overlap* overlap = nullptr;

        if (reader->next(&overlap)) {
            overlaps.emplace_back(overlap);
        }
    }

    delete reader;
    f.close();

    timer.stop();
    timer.print("IO", "afg input");
}

void writeAfgOverlaps(const std::vector<Overlap*>& overlaps, const char* path) {

    Timer timer;
    timer.start();

    std::ofstream f(path);

    for (const auto& overlap : overlaps) {
        f << "{OVL" << std::endl;
        f << "adj:" << (overlap->isInnie() ? "I" : "N") << std::endl;
        f << "rds:" << overlap->getA() << "," << overlap->getB() << std::endl;
        f << "scr:" << overlap->getLength() << std::endl;
        f << "ahg:" << overlap->getAHang() << std::endl;
        f << "bhg:" << overlap->getBHang() << std::endl;
        f << "}" << std::endl;
    }

    f.close();

    timer.stop();
    timer.print("IO", "afg output");
}

bool fileExists(const char* path) {
    struct stat buf;
    return stat(path, &buf) != -1;
}

void fileRead(char** bytes, const char* path) {

    FILE* f = fileSafeOpen(path, "rb");

    size_t bytesLen;

    ASSERT(fread(&bytesLen, sizeof(bytesLen), 1, f) == 1, "IO", "reading failed");

    *bytes = new char[bytesLen];
    ASSERT(fread(*bytes, sizeof(**bytes), bytesLen, f) == bytesLen, "IO", "reading failed");

    fclose(f);
}

void fileWrite(const char* bytes, size_t bytesLen, const char* path) {

    FILE* f = fileSafeOpen(path, "wb");

    ASSERT(fwrite(&bytesLen, sizeof(bytesLen), 1, f) == 1, "IO", "writing failed");
    ASSERT(fwrite(bytes, sizeof(*bytes), bytesLen, f) == bytesLen, "IO", "writing failed");

    fclose(f);
}
