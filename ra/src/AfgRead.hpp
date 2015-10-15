/*
* AfgRead.hpp
*
* Created on: Apr 20, 2015
*     Author: rvaser
*/

#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

class AfgRead: public Read {
public:

    AfgRead(int id, const std::string& name, const std::string& sequence, const std::string& quality, double coverage);
    ~AfgRead() {};

    int getId() const {
        return id_;
    }

    void setId(uint read_id) {
      id_ = read_id;
    }

    const std::string& getName() const {
        return name_;
    }

    const std::string& getSequence() const {
        return sequence_;
    }

    size_t getLength() const {
        return sequence_.size();
    }

    const std::string& getQuality() const {
        return quality_;
    }

    const std::string& getReverseComplement() const {
        return reverseComplement_;
    }

    double getCoverage() const {
        return coverage_;
    }

    void addCoverage(double value) {
        coverage_ += value;
    }

    AfgRead* clone() const;

    void correctBase(int idx, int c);

    void createReverseComplement();

    void serialize(char** bytes, uint32_t* bytes_length) const;

    static AfgRead* deserialize(const char* bytes);

private:

    AfgRead() {};

    uint32_t id_;
    std::string name_;
    std::string sequence_;
    std::string quality_;
    double coverage_;
    std::string reverseComplement_;
};
