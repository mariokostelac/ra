
#pragma once

#include "Contig.hpp"
#include "StringGraph.hpp"

class ContigExtractor {
public:
    ContigExtractor(StringGraphComponent* component) : component_(component) {}

    Contig* extractContig();

private:
    StringGraphComponent* component_;

    int longestPath(int vertexId, bool usePrefix,
        std::unordered_map<std::pair<int, bool>, int>& cache,
        std::vector<bool>& visited);
};
