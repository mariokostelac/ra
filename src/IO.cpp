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
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

Options::Options(const char* readsPath, int threadLen, int k, int c) :
    readsPath(readsPath), threadLen(threadLen), k(k), c(c) {
}

Options* Options::parseOptions(int argc, char** argv) {

    char* readsPath = NULL;
    int threadLen = std::max(std::thread::hardware_concurrency(), 1U);
    int k = -1;
    int c = -1;

    while (1) {

        char argument = getopt_long(argc, argv, "i:t:h", options_, NULL);

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
        default:
            help();
            return NULL;
        }
    }

    ASSERT(readsPath, "IO", "missing option -i (reads file)");
    ASSERT(threadLen >= 0, "IO", "invalid thread number");

    return new Options(readsPath, threadLen, k, c);
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
    "    -kmer <int>\n"
    "        default: based on dataset\n"
    "        length of k-mers used in error correction\n"
    "    -threshold <int>\n"
    "        default: based on dataset\n"
    "        minimal number of occurrences for a k-mer to not be erroneous\n"
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

    while (!feof(f)) {

        int readLen = fread(buffer, sizeof(char), BUFFER_SIZE, f);

        for (int i = 0; i < readLen; ++i) {

            if (buffer[i] == '>') {

                if (createRead) {
                    reads.push_back(new Read(name, sequence));
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

    reads.push_back(new Read(name, sequence));

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

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        switch (i % 4) {
            case 0:
                if (i != 0) {
                    reads.push_back(new Read(name, sequence));
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

    reads.push_back(new Read(name, sequence));

    f.close();

    timer.stop();
    timer.print("IO", "fastq input");
}

void readAfgReads(std::vector<Read*>& reads, const char* path) {

}

void readFromFile(char** bytes, const char* path) {

    FILE* f = fileSafeOpen(path, "rb");

    size_t bytesLen;

    ASSERT(fread(&bytesLen, sizeof(bytesLen), 1, f) == 1, "IO", "reading failed");

    *bytes = new char[bytesLen];
    ASSERT(fread(*bytes, sizeof(**bytes), bytesLen, f) == bytesLen, "IO", "reading failed");

    fclose(f);
}

void writeToFile(const char* bytes, size_t bytesLen, const char* path) {

    FILE* f = fileSafeOpen(path, "wb");

    ASSERT(fwrite(&bytesLen, sizeof(bytesLen), 1, f) == 1, "IO", "writing failed");
    ASSERT(fwrite(bytes, sizeof(*bytes), bytesLen, f) == bytesLen, "IO", "writing failed");

    fclose(f);
}
