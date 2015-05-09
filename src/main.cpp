#include <stdio.h>
#include <stdlib.h>

#include "Read.hpp"
#include "IO.hpp"
#include "Preprocess.hpp"

int main(int argc, char* argv[]) {

    Options* options = Options::parseOptions(argc, argv);

    if (options == NULL) return -1;

    std::vector<Read*> reads;
    readFastqReads(reads, options->readsPath);

    errorCorrection(reads, options->threadLen, options->readsPath);

    for (const auto& it : reads) {
        delete it;
    }

    return 0;
}
