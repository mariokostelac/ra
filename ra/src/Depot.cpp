/*!
 * @file Depot.cpp
 *
 * @brief Depot class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Oct 14, 2015
 */

#include "Depot.hpp"

constexpr mode_t kPermissions = 0775;

int pathExists(const char* path) {
    struct stat buf;
    return stat(path, &buf);
}

int createFolder(const char* path) {

    int status = 0;

    if (pathExists(path) != 0) {
        if (mkdir(path, kPermissions) != 0 && EEXIST != 0) {
            status = -1;
        }
    }

    return status;
}

int openAndLockFileForRW(FILE** dst, const char* path) {

    if (pathExists(path) != 0) {
        auto file = fopen(path, "wb");
        if (file == nullptr) {
            return -1;
        }
        fclose(file);
    }

    *dst = fopen(path, "rb+");
    if (*dst == nullptr) {
        return -1;
    }

    auto err = flock(fileno(*dst), LOCK_EX | LOCK_NB);
    return err;
}

int unlockAndCloseFile(FILE* file) {
    auto status = flock(fileno(file), LOCK_UN);
    fclose(file);
    return status;
}

Depot::Depot(const std::string& path) {

    ASSERT(createFolder(path.c_str()) == 0, "Depot", "Unable to create depot folder!");

    std::string path_ = path + "/reads_data.bin";
    ASSERT(openAndLockFileForRW(&reads_data_, path_.c_str()) == 0, "Depot",
        "Unable to open and lock reads data file!");

    path_ = path + "/reads_index.bin";
    ASSERT(openAndLockFileForRW(&reads_index_, path_.c_str()) == 0, "Depot",
        "Unable to open and lock reads index file!");

    path_ = path + "/overlaps_data.bin";
    ASSERT(openAndLockFileForRW(&overlaps_data_, path_.c_str()) == 0, "Depot",
        "Unable to open and lock overlaps data file!");

    path_ = path + "/overlaps_index.bin";
    ASSERT(openAndLockFileForRW(&overlaps_index_, path_.c_str()) == 0, "Depot",
        "Unable to open and lock overlaps index file!");
}

Depot::~Depot() {

    unlockAndCloseFile(reads_data_);
}

void Depot::store_reads(const ReadSet& src) const {

}
