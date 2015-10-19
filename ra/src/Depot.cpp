/*!
 * @file Depot.cpp
 *
 * @brief Depot class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Oct 14, 2015
 */

#include "AfgRead.hpp"
#include "DepotObject.hpp"
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

    std::string path_ = path + "/read_data.bin";
    openAndLockFile(&read_data_, path_.c_str());

    path_ = path + "/read_index.bin";
    openAndLockFile(&read_index_, path_.c_str());

    path_ = path + "/overlap_data.bin";
    openAndLockFile(&overlap_data_, path_.c_str());

    path_ = path + "/overlap_index.bin";
    openAndLockFile(&overlap_index_, path_.c_str());
}

Depot::~Depot() {

    unlockAndCloseFile(read_data_);
    unlockAndCloseFile(read_index_);
    unlockAndCloseFile(overlap_data_);
    unlockAndCloseFile(overlap_index_);
}

void Depot::store_reads(const ReadSet& src)  {

    ASSERT(src.size() != 0, "Depot", "Can not store an empty ReadSet!");
    store(src, read_data_, read_index_);
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
    load(dst, begin, length, read_data_, read_index_);
}

void Depot::store_overlaps(const OverlapSet& src) {

    ASSERT(src.size() != 0, "Depot", "Can not store empty OverlapSet!");
    //store(src, overlap_data_, overlap_index_);
}

Overlap* Depot::load_overlap(uint32_t index) {

    OverlapSet temp;
    load_overlaps(temp, index, 1);
    return temp.front();
}

void Depot::load_overlaps(OverlapSet& dst) {
    load_overlaps(dst, 0, -1); // read all
}

void Depot::load_overlaps(OverlapSet& dst, uint32_t begin, uint32_t length) {
    //load(dst, begin, length, overlaps_data_, overlaps_index_);
}

template<typename T>
void Depot::store(const std::vector<T*>& src, FILE* data, FILE* index) {

    std::unique_lock<std::mutex> lock(mutex_);

    fseekWrapper(index, 0, SEEK_SET);
    fseekWrapper(data, 0, SEEK_SET);

    uint32_t offsets_size = src.size() + 2;
    uint64_t* offsets = new uint64_t[offsets_size]();
    offsets[0] = offsets_size - 2;

    size_t data_bytes = 0;
    size_t index_bytes = offsets_size * sizeof(*offsets);

    uint32_t id = 1;
    uint32_t uint32_size = sizeof(uint32_t);

    char* buffer = new char[kBufferSize];
    uint32_t buffer_size = 0;

    for (const auto& it: src) {

        char* bytes = nullptr;
        uint32_t bytes_length = 0;
        it->serialize(&bytes, &bytes_length);

        if (buffer_size + bytes_length > kBufferSize) {
            fwriteWrapper(buffer, sizeof(*buffer), buffer_size, data);
            buffer_size = 0;
        }

        std::memcpy(buffer + buffer_size, &bytes_length, uint32_size);
        buffer_size += uint32_size;

        std::memcpy(buffer + buffer_size, bytes, bytes_length);
        buffer_size += bytes_length;

        delete[] bytes;

        offsets[id + 1] = offsets[id] + bytes_length + uint32_size;
        ++id;
    }

    data_bytes = offsets[offsets_size - 1];

    if (buffer_size > 0) {
        fwriteWrapper(buffer, sizeof(*buffer), buffer_size, data);
    }

    fwriteWrapper(offsets, sizeof(*offsets), offsets_size, index);

    delete[] buffer;
    delete[] offsets;

    ftruncateWraper(index, index_bytes);
    ftruncateWraper(data, data_bytes);
}

template<typename T>
void Depot::load(std::vector<T*>& dst, uint32_t begin, uint32_t length,
    FILE* data, FILE* index) {

    std::unique_lock<std::mutex> lock(mutex_);

    ASSERT(!fileEmpty(index), "Depot",
        "Unable to load from empty index file!");
    ASSERT(!fileEmpty(data), "Depot",
        "Unable to load from empty data file!");

    uint64_t reads_length;
    fseekWrapper(index, 0, SEEK_SET);
    freadWrapper(&reads_length, sizeof(reads_length), 1, index);

    ASSERT(begin < (uint32_t) reads_length, "Depot",
        "Beginning index out of range!");

    length = std::min(length, (uint32_t) reads_length - begin);

    uint64_t* offsets = new uint64_t[length + 1];
    fseekWrapper(index, begin * sizeof(uint64_t), SEEK_CUR);
    freadWrapper(offsets, sizeof(*offsets), length + 1, index);

    fseekWrapper(read_data_, offsets[0], SEEK_SET);

    uint32_t uint32_size = sizeof(uint32_t);

    char* buffer = new char[kBufferSize];
    uint32_t buffer_size = 0;

    for (uint32_t i = 0; i < length; ++i) {

        auto bytes_length = offsets[i + 1] - offsets[i];

        if (buffer_size + bytes_length > kBufferSize) {

            freadWrapper(buffer, sizeof(*buffer), buffer_size, data);

            uint32_t ptr = 0;
            while (ptr < buffer_size) {

                uint32_t bytes_length_;
                std::memcpy(&bytes_length_, buffer + ptr, uint32_size);
                ptr += uint32_size;

                dst.emplace_back(static_cast<T*>(DepotObject::deserialize(buffer + ptr)));
                ptr += bytes_length_;
            }

            buffer_size = 0;
        }

        buffer_size += bytes_length;
    }

    if (buffer_size > 0) {

        freadWrapper(buffer, sizeof(*buffer), buffer_size, data);

        uint32_t ptr = 0;
        while (ptr < buffer_size) {

            uint32_t bytes_length;
            std::memcpy(&bytes_length, buffer + ptr, uint32_size);
            ptr += uint32_size;

            dst.emplace_back(static_cast<T*>(DepotObject::deserialize(buffer + ptr)));
            ptr += bytes_length;
        }
    }

    delete[] buffer;
    delete[] offsets;
}
