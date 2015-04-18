#include <stdio.h>
#include <stdlib.h>

#include "EnhancedSuffixArray.hpp"

int main(int argc, char* argv[]) {

    std::string s = "acaaacatat";

    EnhancedSuffixArray* esa = new EnhancedSuffixArray(s);

    esa->print();

    delete esa;

    return 0;
}
