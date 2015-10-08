
#pragma once

#include "StringGraph.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief Method for "converting" string graph to unitig graph
 * @details Extract unitigs and remove all edges that are incident
 * with unitig reads, but not part of unitig.
 *
 * @param number of removed edges
 */
uint32_t remove_external_unitig_edges(StringGraph* graph);
