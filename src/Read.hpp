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

    Read(const std::string& name, const std::string& sequence);
    
    void createReverseComplement();

    const std::string& getName() const {
        return name_;
    }

    const std::string& getSequence() const {
        return sequence_;
    }

    size_t getLength() const {
        return length_;
    }

    void correctBase(int idx, int c) {
        sequence_[idx] = c;
    }

    const std::string& getReverseComplement() const {
        return reverseComplement_;
    }

private:

    std::string name_;
    std::string sequence_;
    size_t length_;
    std::string reverseComplement_;
};