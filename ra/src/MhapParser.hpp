
#ifndef _MHAP_PARSER_H
#define _MHAP_PARSER_H

#include <iostream>
#include <vector>

#include "Read.hpp"
#include "Overlap.hpp"

using std::istream;
using std::vector;

namespace MHAP {

  int read_overlaps(OverlapSet& dst, const ReadSet& reads, istream& input);

}

#endif
