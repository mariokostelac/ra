/*!
 * @file IO.cpp
 *
 * @brief Input Output methods source file
 *
 * @author: rvaser
 * @date Apr 21, 2015
 */

#include "AfgRead.hpp"
#include "IO.hpp"

#include "../vendor/afgreader/reader.h"

#define BUFFER_SIZE 4096

static FILE* fileSafeOpen(const char* path, const char* mode) {
    FILE* f = fopen(path, mode);
    ASSERT(f != nullptr, "IO", "cannot open file %s with mode %s", path, mode);
    return f;
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
                    reads.push_back(new AfgRead(idx++, name, sequence, "", 1.0));
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

    reads.push_back(new AfgRead(idx, name, sequence, "", 1.0));

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
                    reads.push_back(new AfgRead(idx++, name, sequence, quality, 1.0));
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

    reads.push_back(new AfgRead(idx, name, sequence, quality, 1.0));

    f.close();

    timer.stop();
    timer.print("IO", "fastq input");
}

void readAfgReads(std::vector<Read*>& reads, const char* path) {

    Timer timer;
    timer.start();

    ASSERT(fileExists(path), "IO", "cannot open file %s with mode r", path);

    std::ifstream f(path);

    readAfgReads(reads, f);

    f.close();

    timer.stop();
    timer.print("IO", "afg input");
}

void readAfgReads(std::vector<Read*>& reads, std::istream& input) {

    AMOS::Reader* reader = new AMOS::Reader(input);

    while (reader->has_next()) {

        Read* read = nullptr;

        if (reader->next(&read)) {
            reads.emplace_back(read);
        }
    }

    delete reader;
}

void writeFastaReads(const std::vector<Read*>& reads, const char* path) {

    Timer timer;
    timer.start();

    std::ofstream file;

    if (path != nullptr) file.open(path, std::ios::out);

    std::ostream& out = path == nullptr ? std::cout : file;

    for (const auto& read : reads) {
        out << ">" << read->getName() << std::endl;
        out << read->getSequence() << std::endl;
    }

    if (path != nullptr) file.close();

    timer.stop();
    timer.print("IO", "fasta output");
}

void writeAfgReads(const std::vector<Read*>& reads, const char* path) {

    Timer timer;
    timer.start();

    std::ofstream file;

    if (path != nullptr) file.open(path, std::ios::out);

    std::ostream& out = path == nullptr ? std::cout : file;

    for (const auto& read : reads) {
        out << "{RED" << std::endl;
        out << "clr:0," << read->getLength() << std::endl;
        out << "eid:" << read->getName() << std::endl;
        out << "iid:" << read->getId() << std::endl;
        out << "qlt:" << read->getQuality() << std::endl;
        out << "." << std::endl;
        out << "seq:" << read->getSequence() << std::endl;
        out << "." << std::endl;
        out << "cvg:" << read->getCoverage() << std::endl;
        out << "}" << std::endl;
    }

    if (path != nullptr) file.close();

    timer.stop();
    timer.print("IO", "afg output");
}

void readAfgOverlaps(std::vector<DovetailOverlap*>& overlaps, const char* path) {

    Timer timer;
    timer.start();

    ASSERT(fileExists(path), "IO", "cannot open file %s with mode r", path);

    std::ifstream f(path);
    readAfgOverlaps(overlaps, f);
    f.close();

    timer.stop();
    timer.print("IO", "afg input");
}

void readAfgOverlaps(std::vector<DovetailOverlap*>& overlaps, std::istream& input) {

    AMOS::Reader* reader = new AMOS::Reader(input);

    while (reader->has_next()) {

        AfgOverlap* overlap = nullptr;

        if (reader->next(&overlap)) {
            overlaps.emplace_back(overlap);
        }
    }

    delete reader;
}

void write_overlaps(const std::vector<DovetailOverlap*>& overlaps, const char* path) {
  std::vector<Overlap*> converted(overlaps.size());

  for (int i = 0, n = overlaps.size(); i < n; ++i) {
    converted[i] = overlaps[i];
  }

  write_overlaps(converted, path);
}

void write_overlaps(const std::vector<DovetailOverlap*>& overlaps, const std::string path) {
  write_overlaps(overlaps, path.c_str());
}

void write_overlaps(const std::vector<Overlap*>& overlaps, const std::string path) {
    write_overlaps(overlaps, path.c_str());
}

void write_overlaps(const std::vector<Overlap*>& overlaps, const char* path) {

    Timer timer;
    timer.start();

    std::ofstream file;

    if (path != nullptr) file.open(path, std::ios::out);

    std::ostream& out = path == nullptr ? std::cout : file;

    for (const auto& overlap : overlaps) {
      out << *overlap;
    }

    if (path != nullptr) file.close();

    timer.stop();
    timer.print("IO", "afg output");
}

void readAfgContigs(std::vector<Contig*>& contigs, const char* path) {

    Timer timer;
    timer.start();

    ASSERT(fileExists(path), "IO", "cannot open file %s with mode r", path);

    std::ifstream f(path);
    std::string line;

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        if (std::strncmp(line.c_str(), "{LAY", 4) == 0) {
            // found contig;
            bool end = true;
            Contig* contig = new Contig();

            int lo, hi, off, id;

            while (std::getline(f, line)) {

                if (std::strncmp(line.c_str(), "{TLE", 4) == 0) {
                    end = false;
                    // skip
                } else if (sscanf(line.c_str(), "clr:%d, %d", &lo, &hi) == 2) {
                    // skip
                } else if (sscanf(line.c_str(), "off:%d", &off) == 1) {
                    // skip
                } else if (sscanf(line.c_str(), "src:%d", &id) == 1) {
                    // skip
                } else if (std::strncmp(line.c_str(), "}", 1) == 0) {
                    if (end == true) {
                        break;
                    }
                    end = true;
                    contig->addPart(ContigPart(id, lo, hi, off));
                }
            }

            ASSERT(contig->getParts().size() > 0, "IO", "invalid contig data");

            contigs.emplace_back(contig);
        }
    }

    f.close();

    timer.stop();
    timer.print("IO", "afg input");
}

void writeAfgContigs(const std::vector<Contig*>& contigs, const char* path) {

    Timer timer;
    timer.start();

    std::ofstream file;

    if (path != nullptr) file.open(path, std::ios::out);

    std::ostream& out = path == nullptr ? std::cout : file;

    for (const auto& contig : contigs) {

        out << "{LAY" << std::endl;

        for (const auto& part : contig->getParts()) {

            out << "{TLE" << std::endl;
            out << "clr:" << part.clr_lo << "," << part.clr_hi << std::endl;
            out << "off:" << part.offset << std::endl;
            out << "src:" << part.src << std::endl;
            out << "rvc:" << part.type() << std::endl;
            out << "}" << std::endl;
        }

        out << "}" << std::endl;
    }

    if (path != nullptr) file.close();

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

void read_dovetail_overlaps(std::vector<DovetailOverlap*>* overlaps, FILE* fd) {
  int a, b, a_hang, b_hang;
  char type;

  while (fscanf(fd, "%d %d %c %d %d", &a, &b, &type, &a_hang, &b_hang) == 5) {

    assert(a > 0);
    assert(b > 0);
    assert(type == 'N' || type == 'I');

    overlaps->emplace_back(new DovetailOverlap(a, b, a_hang, b_hang, type == 'I'));
  }
}

FILE* must_fopen(const std::string path, const char* mode) {
  return must_fopen(path.c_str(), mode);
}

FILE* must_fopen(const char* path, const char* mode) {
  FILE* res = fopen(path, mode);
  if (res == nullptr) {
    fprintf(stderr, "Cannot open %s with mode %s\n", path, mode);
    exit(1);
  }

  return res;
}

