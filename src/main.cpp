#include <stdio.h>
#include <stdlib.h>

#include "EnhancedSuffixArray.hpp"

int main(int argc, char* argv[]) {

    std::string s = "acatatacatat";
    std::vector<std::string> ss;
    ss.push_back(s);
    ss.push_back(s);
    ss.push_back(s);
    ss.push_back(s);
    ss.push_back(s);

    EnhancedSuffixArray* esa = new EnhancedSuffixArray(ss);

    esa->print();
    printf("tat#\n");
    printf("Number of occurrences: %d\n", esa->getNumberOfOccurrences("tat#"));

    std::vector<int> positions;
    esa->getOccurrences(positions, "tat#");

    printf("Positions: ");

    for (int i = 0; i < (int) positions.size(); ++i) {
        printf("%d ", positions[i]);
    }
    printf("\n");

    delete esa;

    return 0;
}
