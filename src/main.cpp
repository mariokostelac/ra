#include <stdio.h>
#include <stdlib.h>

#include "EnhancedSuffixArray.hpp"
#include "Read.hpp"
#include "IO.hpp"

int main(int argc, char* argv[]) {

    IO* io = new IO();
    std::vector<Read*> reads;

    io->readFastqReads(reads, "test2.fa");

    for (const auto& it : reads) {
        it->createReverseComplement();
    }

    EnhancedSuffixArray* index1 = new EnhancedSuffixArray(reads);

    // index1->print();

    delete index1;

    for (const auto& it : reads) {
        delete it;
    }

    delete io;

    return 0;
}
