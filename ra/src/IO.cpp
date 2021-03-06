/*!
 * @file IO.cpp
 *
 * @brief Input Output methods source file
 *
 * @author: rvaser
 * @date Apr 21, 2015
 */

#include "IO.hpp"

#include "../vendor/afgreader/reader.h"

#define BUFFER_SIZE 4096

static FILE* fileSafeOpen(const char* path, const char* mode) {
    FILE* f = fopen(path, mode);
    ASSERT(f != nullptr, "IO", "cannot open file %s with mode %s", path, mode);
    return f;
}

void readFastaReads(ReadSet& reads, const char* path) {

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

void readFastqReads(ReadSet& reads, const char* path) {

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

void readAfgReads(ReadSet& reads, const char* path) {

    Timer timer;
    timer.start();

    ASSERT(fileExists(path), "IO", "cannot open file %s with mode r", path);

    std::ifstream f(path);

    readAfgReads(reads, f);

    f.close();

    timer.stop();
    timer.print("IO", "afg input");
}

void readAfgReads(ReadSet& reads, std::istream& input) {

    AMOS::Reader* reader = new AMOS::Reader(input);

    while (reader->has_next()) {

        Read* read = nullptr;

        if (reader->next(&read)) {
            reads.emplace_back(read);
        }
    }

    delete reader;
}

void writeFastaReads(const ReadSet& reads, const char* path) {

    Timer timer;
    timer.start();

    std::ofstream file;

    if (path != nullptr) file.open(path, std::ios::out);

    std::ostream& out = path == nullptr ? std::cout : file;

    for (const auto& read : reads) {
        out << ">" << read->name() << std::endl;
        out << read->sequence() << std::endl;
    }

    if (path != nullptr) file.close();

    timer.stop();
    timer.print("IO", "fasta output");
}

void writeAfgReads(const ReadSet& reads, const char* path) {

    Timer timer;
    timer.start();

    std::ofstream file;

    if (path != nullptr) file.open(path, std::ios::out);

    std::ostream& out = path == nullptr ? std::cout : file;

    for (const auto& read : reads) {
        out << "{RED" << std::endl;
        out << "clr:0," << read->length() << std::endl;
        out << "eid:" << read->name() << std::endl;
        out << "iid:" << read->id() << std::endl;
        out << "qlt:" << read->quality() << std::endl;
        out << "." << std::endl;
        out << "seq:" << read->sequence() << std::endl;
        out << "." << std::endl;
        out << "cvg:" << read->coverage() << std::endl;
        out << "}" << std::endl;
    }

    if (path != nullptr) file.close();

    timer.stop();
    timer.print("IO", "afg output");
}

void readAfgOverlaps(OverlapSet& overlaps, const ReadSet& reads, const char* path) {

    Timer timer;
    timer.start();

    ASSERT(fileExists(path), "IO", "cannot open file %s with mode r", path);

    std::ifstream f(path);
    readAfgOverlaps(overlaps, reads, f);
    f.close();

    timer.stop();
    timer.print("IO", "afg input");
}

void readAfgOverlaps(OverlapSet& overlaps, const ReadSet& reads, std::istream& input) {

    AMOS::Reader* reader = new AMOS::Reader(input);

    while (reader->has_next()) {

        Overlap* overlap = nullptr;

        if (reader->next(&overlap, reads)) {
            overlaps.emplace_back(overlap);
        }
    }

    delete reader;
}

void write_overlaps(const OverlapSet& overlaps, const std::string path) {
  write_overlaps(overlaps, path.c_str());
}

void write_overlaps(const OverlapSet& overlaps, const char* path) {

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

void read_dovetail_overlaps(OverlapSet& overlaps, const ReadSet& reads, FILE* fd) {
  int a, b, a_hang, b_hang;
  char type;
  double orig_errate, errate;

  while (fscanf(fd, "%d %d %c %d %d %lf %lf", &a, &b, &type, &a_hang, &b_hang, &orig_errate, &errate) == 7) {

    assert(a > 0 && a < reads.size());
    assert(b > 0 && b < reads.size());
    assert(type == 'N' || type == 'I');

    overlaps.emplace_back(new Overlap(reads[a], a_hang, reads[b], b_hang,
        type == 'I', errate, orig_errate));
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

void writeRadumpOverlaps(FILE* dst, OverlapSet& overlaps) {
  for (auto o : overlaps) {
    fprintf(stdout, "%d\t%d\t%c\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\t\n",
        o->a(), o->b(), o->is_innie() ? 'I' : 'N',
        o->a_lo(), o->a_hi(), o->read_a()->length(),
        o->b_lo(), o->b_hi(), o->read_b()->length(),
        o->orig_err_rate(), o->err_rate()
        );
  }
}

void readRadumpOverlaps(OverlapSet* overlaps, ReadSet& reads, FILE* src) {
  while (true) {
    // direct values
    int a_id, b_id;
    uint32_t a_lo, a_hi, a_len, b_lo, b_hi, b_len;
    double err_rate, orig_err_rate;
    char type;

    int read = fscanf(src, " %d %d %c %d %d %d %d %d %d %lf %lf ",
        &a_id, &b_id, &type,
        &a_lo, &a_hi, &a_len,
        &b_lo, &b_hi, &b_len,
        &orig_err_rate, &err_rate
        );
    if (read != 11) {
      break;
    }

    assert("a_id in range" && (a_id >= 0 && a_id < reads.size()));
    assert("b_id in range" && (b_id >= 0 && b_id < reads.size()));
    assert("type N or I" && (type == 'N' || type == 'I'));

    // calculated values
    bool a_rc = false;
    bool b_rc = type == 'I';
    Read* read_a = reads[a_id];
    Read* read_b = reads[b_id];

    Overlap* o = new Overlap(read_a, a_lo, a_hi, a_rc,
        read_b, b_lo, b_hi, b_rc,
        err_rate, orig_err_rate);
    overlaps->push_back(o);
  }
}
