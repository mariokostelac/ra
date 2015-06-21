
#ifndef _AMOS_READER_H
#define _AMOS_READER_H

#include "../../src/AfgRead.hpp"
#include "../../src/AfgOverlap.hpp"

#include <cstdio>
#include <vector>
#include <stack>

namespace AMOS {

  enum ReaderState {
    OUT,
    IN,
    IN_STR
  };

  enum ObjectType {
    CONTIG,
    CONTIG_EDGE,
    CONTIGlINK,
    DISTRIBUTION,
    EDGE,
    FEATURE,
    FRAGMENT,
    GROUP,
    ID_MAP,
    INDEX,
    KMER,
    LAYOUT,
    LIBRARY,
    LINK,
    OVERLAP,
    READ,
    SCAFFOLD,
    SCAFFOLD_EDGE,
    SCAFFOLD_LINK,
    SEQUENCE,
    UNIVERSAL
  };

  enum MarkType {
    ObjectDef,
    ObjectEnd,
    AttrDef
  };

  class BufferMark {
    public:
    MarkType type;
    int lo;
    int hi;

    BufferMark(MarkType type, int lo, int hi): type(type), lo(lo), hi(hi) {}
  };

  class Reader {
    public:
      Reader(std::istream& input);
      ~Reader();
      bool has_next();
      int skip_next();
      ObjectType next_type();
      bool next(Read** dst);
      bool next(Overlap** dst);

    private:
      std::istream* input;
      char* buff;
      int buff_cap;
      int buff_written;
      std::vector<BufferMark> buff_marks;
      std::stack<ReaderState> states;
      ObjectType next_type_;

      int buffer_next_line();
      int buffer_next();
      int buffer_double();
      int buffer_clear();
      bool read_from_buff(Read** dst);
      bool overlap_from_buff(Overlap** dst);
  };
}

#endif
