/*!
 * @file Depot.cpp
 *
 * @brief Depot class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Oct 14, 2015
 */

#include "AfgRead.hpp"
#include "Depot.hpp"

constexpr mode_t kPermissions = 0775;
constexpr uint32_t kBufferSize = 1024 * 1024 * 1024;

FILE* fopenWrapper(const char* file_name, const char* mode) {
    auto file = fopen(file_name, mode);
    ASSERT(file != nullptr, "Depot",
        "Unable to open file %s with mode %s (fopen)!", file_name, mode);
    return file;
}

void fseekWrapper(FILE* stream, long int offset, int origin) {
    ASSERT(fseek(stream, offset, origin) == 0, "Depot",
        "Unable to change position in file (fseek)!");
}

void freadWrapper(void* ptr, size_t size, size_t count, FILE* stream) {
    ASSERT(fread(ptr, size, count, stream) == count, "Depot",
        "Unable to read from file (fread)!");
}

void fwriteWrapper(void *ptr, size_t size, size_t count, FILE* stream) {
    ASSERT(fwrite(ptr, size, count, stream) == count, "Depot",
        "Unable to write to file (fwrite)!");
}

void flockWrapper(FILE* file, int operation) {
    ASSERT(flock(fileno(file), operation) == 0, "Depot",
        "Unable to (un)lock file (flock)!");
}

void ftruncateWraper(FILE* file, off_t length) {
    ASSERT(ftruncate(fileno(file), length) == 0, "Depot",
        "Unable to truncate file (ftruncate)!");
}

bool fileEmpty(FILE* file) {

    fseekWrapper(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        return true;
    }
    return false;
}

int pathExists(const char* path) {
    struct stat buf;
    return stat(path, &buf);
}

void createFolder(const char* path) {
    if (pathExists(path) != 0) {
        ASSERT(!(mkdir(path, kPermissions) != 0 && EEXIST != 0), "Depot",
            "Unable to create depot folder!");
    }
}

void openAndLockFile(FILE** dst, const char* path) {

    if (pathExists(path) != 0) {
        auto file = fopenWrapper(path, "wb");
        fclose(file);
    }

    *dst = fopenWrapper(path, "rb+");
    flockWrapper(*dst, LOCK_EX | LOCK_NB);
}

void unlockAndCloseFile(FILE* file) {
    flockWrapper(file, LOCK_UN);
    fclose(file);
}

Depot::Depot(const std::string& path) {

    createFolder(path.c_str());

    std::string path_ = path + "/reads_data.bin";
    openAndLockFile(&reads_data_, path_.c_str());

    path_ = path + "/reads_index.bin";
    openAndLockFile(&reads_index_, path_.c_str());

    path_ = path + "/overlaps_data.bin";
    openAndLockFile(&overlaps_data_, path_.c_str());

    path_ = path + "/overlaps_index.bin";
    openAndLockFile(&overlaps_index_, path_.c_str());
}

Depot::~Depot() {

    unlockAndCloseFile(reads_data_);
    unlockAndCloseFile(reads_index_);
    unlockAndCloseFile(overlaps_data_);
    unlockAndCloseFile(overlaps_index_);
}

void Depot::store_reads(const ReadSet& src)  {

    std::unique_lock<std::mutex> lock(rmutex_);

    ASSERT(src.size() != 0, "Depot", "Can not store empty ReadSet!");

    fseekWrapper(reads_index_, 0, SEEK_SET);
    fseekWrapper(reads_data_, 0, SEEK_SET);

    uint32_t offsets_size = src.size() + 2;
    uint64_t* offsets = new uint64_t[offsets_size]();
    offsets[0] = offsets_size - 2;

    size_t data_total_bytes = 0;
    size_t index_total_bytes = offsets_size * sizeof(*offsets);

    uint32_t id = 1;
    uint32_t size = sizeof(uint32_t);

    char* buffer = new char[kBufferSize];
    uint32_t buffer_size = 0;

    for (const auto& it: src) {

        char* bytes;
        uint32_t bytes_length;
        it->serialize(&bytes, &bytes_length);

        if (buffer_size + bytes_length > kBufferSize) {
            fwriteWrapper(buffer, sizeof(*buffer), buffer_size, reads_data_);
            buffer_size = 0;
        }

        std::memcpy(buffer + buffer_size, &bytes_length, size);
        buffer_size += size;

        std::memcpy(buffer + buffer_size, bytes, bytes_length);
        buffer_size += bytes_length;

        delete[] bytes;

        offsets[id + 1] = offsets[id] + bytes_length + size;
        ++id;
    }

    data_total_bytes = offsets[offsets_size - 1];

    if (buffer_size > 0) {
        fwriteWrapper(buffer, sizeof(*buffer), buffer_size, reads_data_);
    }

    fwriteWrapper(offsets, sizeof(*offsets), offsets_size, reads_index_);

    delete[] buffer;
    delete[] offsets;

    ftruncateWraper(reads_index_, index_total_bytes);
    ftruncateWraper(reads_data_, data_total_bytes);
}

Read* Depot::load_read(uint32_t index) {

    ReadSet temp;
    load_reads(temp, index, 1);

    return temp.front();
}

void Depot::load_reads(ReadSet& dst) {
    load_reads(dst, 0, -1); // read all
}

void Depot::load_reads(ReadSet& dst, uint32_t begin, uint32_t length) {

    std::unique_lock<std::mutex> lock(rmutex_);

    ASSERT(!fileEmpty(reads_index_), "Depot",
        "Unable to load from empty reads index file!");
    ASSERT(!fileEmpty(reads_data_), "Depot",
        "Unable to load from empty reads data file!");

    uint64_t reads_length;
    fseekWrapper(reads_index_, 0, SEEK_SET);
    freadWrapper(&reads_length, sizeof(reads_length), 1, reads_index_);

    ASSERT(begin < (uint32_t) reads_length, "Depot", "Beginning index out of range!");

    length = std::min(length, (uint32_t) reads_length);

    uint64_t* offsets = new uint64_t[length + 1];
    fseekWrapper(reads_index_, begin * sizeof(uint64_t), SEEK_CUR);
    freadWrapper(offsets, sizeof(*offsets), length + 1, reads_index_);

    fseekWrapper(reads_data_, offsets[0], SEEK_SET);

    uint32_t size = sizeof(uint32_t);

    char* buffer = new char[kBufferSize];
    uint32_t buffer_size = 0;

    for (uint32_t i = 0; i < length; ++i) {

        auto bytes_length = offsets[i + 1] - offsets[i];

        if (buffer_size + bytes_length > kBufferSize) {

            freadWrapper(buffer, sizeof(*buffer), buffer_size, reads_data_);

            uint32_t ptr = 0;
            while (ptr < buffer_size) {

                uint32_t bytes_length_;
                std::memcpy(&bytes_length_, buffer + ptr, size);
                ptr += size;

                dst.emplace_back(AfgRead::deserialize(buffer + ptr));
                ptr += bytes_length_;
            }

            buffer_size = 0;
        }

        buffer_size += bytes_length;
    }

    if (buffer_size > 0) {

        freadWrapper(buffer, sizeof(*buffer), buffer_size, reads_data_);

        uint32_t ptr = 0;
        while (ptr < buffer_size) {

            uint32_t bytes_length;
            std::memcpy(&bytes_length, buffer + ptr, size);
            ptr += size;

            dst.emplace_back(AfgRead::deserialize(buffer + ptr));
            ptr += bytes_length;
        }
    }

    delete[] buffer;
    delete[] offsets;
}
