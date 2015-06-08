/*
* Read.hpp
*
* Created on: Apr 20, 2015
*     Author: rvaser
*/

#pragma once

#include "CommonHeaders.hpp"

class Read {
public:

    Read(int id, const std::string& name, const std::string& sequence, const std::string& quality, double coverage);
    ~Read() {};

    int getId() const {
        return id_;
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

    Read* clone() const;

    void correctBase(int idx, int c);

    void createReverseComplement();

private:

    int id_;
    std::string name_;
    std::string sequence_;
    std::string quality_;
    double coverage_;
    std::string reverseComplement_;
};