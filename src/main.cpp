#include <stdio.h>
#include <stdlib.h>

#include "SuffixTree.hpp"

int main(int argc, char* argv[]) {

    std::string s = "mississippi~";

    SuffixTree* sf = new SuffixTree(s);

    delete sf;

    return 0;
}
