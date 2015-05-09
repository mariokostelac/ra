#include <stdio.h>
#include <stdlib.h>

#include "Read.hpp"
#include "IO.hpp"
#include "Preprocess.hpp"

int main(int argc, char* argv[]) {

    std::vector<Read*> reads;

    int threadLen = std::thread::hardware_concurrency();
    if (threadLen == 0) threadLen = 1;
    printf("thredLen = %d\n", threadLen);

    readFastqReads(reads, "test2.fa");

    errorCorrection(reads, 15, 4, threadLen, "test2.fa");

    for (const auto& it : reads) {
        delete it;
    }

    return 0;
}
