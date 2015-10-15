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
        "Unable to (un)lock file (lock)!");
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

void Depot::store_reads(const ReadSet& src) const {

    if (src.size() == 0) {
        return;
    }

    fseekWrapper(reads_index_, 0, SEEK_SET);
    fseekWrapper(reads_data_, 0, SEEK_SET);

    size_t data_total_baytes = 0;
    size_t index_total_bytes = (src.size() + 1) * sizeof(uint64_t);

    std::vector<uint64_t> offsets(src.size() + 2, 0);
    uint32_t id = 1;

    uint32_t size = sizeof(uint32_t);

    for (const auto& it: src) {

        ++offsets[0];

        char* bytes;
        uint32_t bytes_length;
        it->serialize(&bytes, &bytes_length);

        offsets[id + 1] = bytes_length + size;
        data_total_baytes += bytes_length + size;

        fwriteWrapper(&bytes_length, size, 1, reads_data_);
        fwriteWrapper(bytes, 1, bytes_length, reads_data_);
        delete[] bytes;
    }

    fwriteWrapper(offsets.data(), sizeof(uint64_t), offsets.size(), reads_index_);

    ftruncateWraper(reads_index_, index_total_bytes);
    ftruncateWraper(reads_data_, data_total_baytes);
}

void Depot::load_reads(ReadSet& dst) const {

    if (fileEmpty(reads_index_) || fileEmpty(reads_data_)) {
        return;
    }

    load_reads(dst, 0, -1); // read all
}

void Depot::load_reads(ReadSet& dst, uint32_t begin, uint32_t length) const {

    if (fileEmpty(reads_index_) || fileEmpty(reads_data_)) {
        return;
    }

    uint64_t reads_length;
    fseekWrapper(reads_index_, 0, SEEK_SET);
    freadWrapper(&reads_length, sizeof(reads_length), 1, reads_index_);

    if (begin >= (uint32_t) reads_length) {
        return;
    }
    length = std::min(length, (uint32_t) reads_length);

    uint64_t data_offset;
    fseekWrapper(reads_index_, begin * sizeof(data_offset), SEEK_CUR);
    freadWrapper(&data_offset, sizeof(data_offset), 1, reads_index_);

    fseekWrapper(reads_data_, data_offset, SEEK_SET);

    uint32_t size = sizeof(uint32_t);

    for (uint32_t i = 0; i < length; ++i) {

        uint32_t bytes_length;
        freadWrapper(&bytes_length, size, 1, reads_data_);

        char buffer[bytes_length];
        freadWrapper(buffer, 1, bytes_length, reads_data_);

        dst.emplace_back(AfgRead::deserialize(buffer));
    }
}

Read* Depot::load_read(uint32_t index) const {

    if (fileEmpty(reads_index_) || fileEmpty(reads_data_)) {
        return nullptr;
    }

    ReadSet temp;
    load_reads(temp, index, 1);

    return temp.empty() ? nullptr : temp.front();
}
