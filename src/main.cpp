#include <stdio.h>
#include <stdlib.h>

#include "EnhancedSuffixArray.hpp"
#include "Read.hpp"
#include "IO.hpp"

int main(int argc, char* argv[]) {

    IO* io = new IO();
    std::vector<Read*> reads;

    io->readFastaReads(reads, "test.fa");

    EnhancedSuffixArray* esa1 = new EnhancedSuffixArray(reads, 0, 0);
    // esa1->print();

    EnhancedSuffixArray* esa2 = new EnhancedSuffixArray(reads, 0, 1);
    // esa2->print();

    std::vector<int> positions1;
    esa1->getOccurrences(positions1, "ACA");

    std::vector<int> positions2;
    esa2->getOccurrences(positions2, "ACA");

    if (positions1.size() != positions2.size()) printf("FAILED\n");
    else {
        bool failed = false;

        for (int i = 0; i < (int) positions1.size(); ++i) {
            if (positions1[i] != positions2[i]) {
                printf("FAILED\n");
                failed = true;
                break;
            }
        }

        if (!failed) printf("PASSED\n");
    }

    delete esa2;
    delete esa1;

    for (const auto& it : reads) {
        delete it;
    }

    delete io;

    return 0;
}
