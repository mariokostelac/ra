#include <stdio.h>
#include <stdlib.h>

#include "SuffixTree.hpp"

int main(int argc, char* argv[]) {

    std::string s = "mississippi~";
    std::vector<int> sa;

    SuffixTree* sf = new SuffixTree(s);

    sf->toSuffixArray(sa);

    for (int i = 0; i < (int) sa.size(); ++i) {
        printf("%d ", sa[i]);
    }
    printf("\n");

    delete sf;

    return 0;
}
