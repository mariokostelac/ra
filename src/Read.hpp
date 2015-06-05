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

    Read(int id, const std::string& name, const std::string& sequence);
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

    int getLength() const {
        return length_;
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
    int length_;
    std::string reverseComplement_;
    double coverage_;
};