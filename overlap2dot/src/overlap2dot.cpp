#include <string>

using std::string;

const string get_edge_style(const bool use_start, const bool use_end) {
  if (use_start && use_end) {
    return "box";
  } else if (use_start) {
    return "dot";
  } else if (use_end) {
    return "odot";
  }

  return "none";
}

