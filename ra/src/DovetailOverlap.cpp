/*!
 * @file DovetailOverlap.cpp
 *
 * @brief DovetailOverlap class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 14, 2015
 */


#include "DovetailOverlap.hpp"

const double EPSILON = 0.15;
const double ALPHA = 3;

static inline bool doubleEq(double x, double y, double eps) {
    return y <= x + eps && x <= y + eps;
}

const std::string overlap("=======");
std::string DovetailOverlap::repr() const {
  std::string a = overlap;
  std::string b = overlap;

  char buff[256];
  if (a_hang() > 0) {
    sprintf(buff, "%-7d", a_hang());
    a = "=======" + a;
    b = std::string(buff) + b;
  } else if (a_hang() < 0) {
    sprintf(buff, "%-7d", a_hang());
    a = std::string(buff) + a;
    b = "=======" + b;
  }

  if (b_hang() > 0) {
    sprintf(buff, "%7d", b_hang());
    a = a + std::string(buff);
    b = b + "=======";
  } else if  (b_hang() < 0) {
    sprintf(buff, "%7d", b_hang());
    a = a + "=======";
    b = b + std::string(buff);
  }

  for (int i = a.size() - 1; i >= 0; i--) {
    if (a[i] == '=') {
      a[i] = '>';
      break;
    }
  }

  if (innie()) {
    for (uint32_t i = 0; i < b.size(); i++) {
      if (b[i] == '=') {
        b[i] = '<';
        break;
      }
    }
  } else {
    for (int i = b.size(); i >= 0; i--) {
      if (b[i] == '=') {
        b[i] = '>';
        break;
      }
    }
  }

  return a + " " + std::to_string(this->a()) + "\n" + b + " " + std::to_string(this->b()) + "\n";
}

bool DovetailOverlap::is_using_prefix(int readId) const {

    if (readId == a()) {
        if (a_hang() <= 0) return true;

    } else if (readId == b()) {
        if (innie_ == false && a_hang() >= 0) return true;
        if (innie_ == true && b_hang() <= 0) return true;
    }

    return false;
}

bool DovetailOverlap::is_using_suffix(int readId) const {

    if (readId == a()) {
        if (b_hang() >= 0) return true;

    } else if (readId == b()) {
        if (innie_ == false && b_hang() <= 0) return true;
        if (innie_ == true && a_hang() >= 0) return true;
    }

    return false;
}


uint DovetailOverlap::hangingLength(int readId) const {

    if (readId == a()) return abs(a_hang());
    if (readId == b()) return abs(b_hang());

    ASSERT(false, "Overlap", "wrong read id");
}


bool DovetailOverlap::is_transitive(const DovetailOverlap* o2, const DovetailOverlap* o3) const {

    auto o1 = this;

    int a_id = o1->a();
    int b_id = o1->b();
    int c_id = o2->a() != a_id ? o2->a() : o2->b();

    if (o2->is_using_suffix(c_id) == o3->is_using_suffix(c_id)) return false;
    if (o1->is_using_suffix(a_id) != o2->is_using_suffix(a_id)) return false;
    if (o1->is_using_suffix(b_id) != o3->is_using_suffix(b_id)) return false;

    if (!doubleEq(
            o2->hangingLength(a_id) + o3->hangingLength(c_id),
            o1->hangingLength(a_id),
            EPSILON * o1->length() + ALPHA)) {
        return false;
    }

    if (!doubleEq(
            o2->hangingLength(c_id) + o3->hangingLength(b_id),
            o1->hangingLength(b_id),
            EPSILON * o1->length() + ALPHA)) {
        return false;
    }

    debug("ISTRAN %d %d because of %d %d and %d %d\n",
        o1->a(),
        o1->b(),
        o2->a(),
        o2->b(),
        o3->a(),
        o3->b()
    );

    return true;
}
