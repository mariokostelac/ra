/*
* AfgRead.hpp
*
* Created on: Apr 20, 2015
*     Author: rvaser
*/

#pragma once

#include "CommonHeaders.hpp"

class Read {
public:

    virtual ~Read(){};

    virtual int getId() const = 0;

    virtual void setId(uint read_id) = 0;

    virtual const std::string& getName() const = 0;

    virtual const std::string& getSequence() const = 0;

    virtual size_t getLength() const = 0;

    virtual const std::string& getQuality() const = 0;

    virtual const std::string& getReverseComplement() const = 0;

    virtual double getCoverage() const = 0;

    virtual void addCoverage(double value) = 0;

    virtual Read* clone() const = 0;

    virtual void correctBase(int idx, int c) = 0;

    virtual void createReverseComplement() = 0;
};

void createReverseComplements(std::vector<Read*>& reads, int threadLen);
