
#ifndef _MHAP_PARSER_H
#define _MHAP_PARSER_H

#include <iostream>
#include <vector>
#include "MhapOverlap.hpp"

using std::istream;
using std::vector;

namespace MHAP {

  int read_overlaps(istream& input, vector<Overlap*>* overlaps);

}

#endif
