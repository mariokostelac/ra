#pragma once

#include "CommonHeaders.hpp"

// trimming params
extern int READ_LEN_THRESHOLD;
extern uint32_t MAX_READS_IN_TIP;
extern uint32_t MAX_DEPTH_WITHOUT_EXTRA_FORK;

// BFS params in bubble popping
extern size_t MAX_NODES;
extern int MAX_DISTANCE;
extern double MAX_DIFFERENCE;

// contig extraction params
extern size_t MAX_BRANCHES;
extern size_t MAX_START_NODES;
extern double LENGTH_THRESHOLD;
extern double QUALITY_THRESHOLD;
