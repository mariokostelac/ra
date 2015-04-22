/*
* IO.hpp
*
* Created on: Apr 21, 2015
*     Author: rvaser
*/

#include <sys/stat.h>

#include "IO.hpp"

#define BUFFER_SIZE 4096

static FILE* fileSafeOpen(const char* path, const char* mode) {
    FILE* f = fopen(path, mode);
    ASSERT(f != NULL, "cannot open file %s with mode %s", path, mode);
    return f;
}

static bool fileExists(const char* path) {
    struct stat buf;
    return stat(path, &buf) != -1;
}

void IO::readFastaReads(std::vector<Read*>& reads, const char* path) {

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
}

void IO::readFastqReads(std::vector<Read*>& reads, const char* path) {

    ASSERT(fileExists(path), "cannot open file %s with mode r", path);

    std::ifstream f(path);
    std::string line;

    std::string name;
    std::string sequence;
    std::string quality;

    int i = 0;

    while (std::getline(f, line)) {

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
}

void IO::readAfgReads(std::vector<Read*>& reads, const char* path) {

}
