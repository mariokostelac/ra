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

bool DovetailOverlap::is_using_prefix(uint32_t read_id) const {

    if (read_id == a()) {
        if (a_hang() <= 0) return true;

    } else if (read_id == b()) {
        if (innie_ == false && a_hang() >= 0) return true;
        if (innie_ == true && b_hang() <= 0) return true;
    }

    return false;
}

bool DovetailOverlap::is_using_suffix(uint32_t read_id) const {

    if (read_id == a()) {
        if (b_hang() >= 0) return true;

    } else if (read_id == b()) {
        if (innie_ == false && b_hang() <= 0) return true;
        if (innie_ == true && a_hang() >= 0) return true;
    }

    return false;
}


uint DovetailOverlap::hanging_length(uint32_t read_id) const {

    if (read_id == a()) {
      int hanging = 0;
      if (a_hang() > 0) {
        hanging += a_hang();
      }
      if (b_hang() < 0) {
        hanging += abs(b_hang());
      }

      return hanging;
    } else if (read_id == b()) {
      int hanging = 0;
      if (a_hang() < 0) {
        hanging += abs(a_hang());
      }
      if (b_hang() > 0) {
        hanging += b_hang();
      }

      return hanging;
    }

    ASSERT(false, "Overlap", "wrong read id");
}


bool DovetailOverlap::is_transitive(const DovetailOverlap* o2, const DovetailOverlap* o3) const {

    auto o1 = this;

    auto a = o1->a();
    auto b = o1->b();
    auto c = o2->a() != a ? o2->a() : o2->b();

    if (o2->is_using_suffix(c) == o3->is_using_suffix(c)) return false;
    if (o1->is_using_suffix(a) != o2->is_using_suffix(a)) return false;
    if (o1->is_using_suffix(b) != o3->is_using_suffix(b)) return false;

    if (!doubleEq(
            o2->hanging_length(a) + o3->hanging_length(c),
            o1->hanging_length(a),
            EPSILON * o1->length() + ALPHA)) {
        return false;
    }

    if (!doubleEq(
            o2->hanging_length(c) + o3->hanging_length(b),
            o1->hanging_length(b),
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

uint32_t DovetailOverlap::a_lo() const {
  if (a_hang() <= 0) {
    return 0;
  }

  return a_hang();
}

uint32_t DovetailOverlap::a_hi() const {
  if (b_hang() >= 0) {
    return read_a()->getLength();
  }

  // b_hang is < 0
  return read_a()->getLength() + b_hang();
}

uint32_t DovetailOverlap::b_lo() const {
  if (a_hang() >= 0) {
    return 0;
  }

  return -a_hang();
}

uint32_t DovetailOverlap::b_hi() const {
  if (b_hang() <= 0) {
    return read_b()->getLength();
  }

  return read_b()->getLength() - b_hang();
}

void DovetailOverlap::print(std::ostream& o) const {
  o << a() << "\t" << b() << "\t" << (innie_ ? 'I' : 'N') << "\t";
  o << a_hang() << "\t" << b_hang()  << "\t" << orig_errate() << "\t";
  o << errate() << std::endl;
}
