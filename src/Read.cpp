/*
* Read.cpp
*
* Created on: Apr 20, 2015
*     Author: rvaser
*/

#include "Read.hpp"

static bool isValidChar(char c) {
    if ((c > 64 && c < 91) || (c > 96 && c < 123)) return true;
    return false;
}

Read::Read(const std::string& name, const std::string& sequence) {

    ASSERT(name.size() > 0 && sequence.size() > 0, "Read", "invalid data");

    name_ = name;

    for (int i = 0; i < (int) sequence.size(); ++i) {
        if (isValidChar(sequence[i])) {
            sequence_ += toupper(sequence[i]);
        }
    }

    ASSERT(sequence_.size() > 0, "Read", "invalid data");
}

void Read::createReverseComplement() {

    for (int i = sequence_.size() - 1; i >= 0; --i) {

        char c = sequence_[i];

        switch (c) {
            case 'A':
                c = 'T';
                break;
            case 'T':
                c = 'A';
                break;
            case 'C':
                c = 'G';
                break;
            case 'G':
                c = 'C';
                break;
            default:
                break;
        }

        reverseComplement_ += c;
    }
}