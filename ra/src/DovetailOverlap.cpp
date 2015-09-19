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

double DovetailOverlap::getScore() const {

  double base_score = (getLength(a_)/(double)ra_->getLength() + getLength(b_)/(double)rb_->getLength());

  return base_score;
}

const std::string overlap("=======");
std::string DovetailOverlap::repr() const {
  std::string a = overlap;
  std::string b = overlap;

  char buff[256];
  if (getAHang() > 0) {
    sprintf(buff, "%-7d", getAHang());
    a = "=======" + a;
    b = std::string(buff) + b;
  } else if (getAHang() < 0) {
    sprintf(buff, "%-7d", getAHang());
    a = std::string(buff) + a;
    b = "=======" + b;
  }

  if (getBHang() > 0) {
    sprintf(buff, "%7d", getBHang());
    a = a + std::string(buff);
    b = b + "=======";
  } else if  (getBHang() < 0) {
    sprintf(buff, "%7d", getBHang());
    a = a + "=======";
    b = b + std::string(buff);
  }

  for (int i = a.size() - 1; i >= 0; i--) {
    if (a[i] == '=') {
      a[i] = '>';
      break;
    }
  }

  if (isInnie()) {
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

  return a + " " + std::to_string(getA()) + "\n" + b + " " + std::to_string(getB()) + "\n";
}

bool DovetailOverlap::isTransitive(const DovetailOverlap* o2, const DovetailOverlap* o3) const {

    auto o1 = this;

    int a_id = o1->getA();
    int b_id = o1->getB();
    int c_id = o2->getA() != a_id ? o2->getA() : o2->getB();

    if (o2->isUsingSuffix(c_id) == o3->isUsingSuffix(c_id)) return false;
    if (o1->isUsingSuffix(a_id) != o2->isUsingSuffix(a_id)) return false;
    if (o1->isUsingSuffix(b_id) != o3->isUsingSuffix(b_id)) return false;

    if (!doubleEq(
            o2->hangingLength(a_id) + o3->hangingLength(c_id),
            o1->hangingLength(a_id),
            EPSILON * o1->getLength() + ALPHA)) {
        return false;
    }

    if (!doubleEq(
            o2->hangingLength(c_id) + o3->hangingLength(b_id),
            o1->hangingLength(b_id),
            EPSILON * o1->getLength() + ALPHA)) {
        return false;
    }

    debug("ISTRAN %d %d because of %d %d and %d %d\n",
        o1->getA(),
        o1->getB(),
        o2->getA(),
        o2->getB(),
        o3->getA(),
        o3->getB()
    );

    return true;
}
