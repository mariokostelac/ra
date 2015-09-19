/*!
 * @file Overlap.cpp
 *
 * @brief Overlap class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 14, 2015
 */


#include "ReadIndex.hpp"

const double EPSILON = 0.15;
const double ALPHA = 3;

static inline bool doubleEq(double x, double y, double eps) {
    return y <= x + eps && x <= y + eps;
}

double Overlap::getScore() const {

  double base_score = (getLength(a_)/(double)ra_->getLength() + getLength(b_)/(double)rb_->getLength());

  return base_score;
}

int Overlap::confirmedLength(const int read_id) const {
    return getLength(read_id);
}

void updateOverlapIds(std::vector<Overlap*>& overlaps, std::vector<Read*>& reads) {

    for (const auto& overlap : overlaps) {
        overlap->setA(reads[overlap->getA()]->getId());
        overlap->setB(reads[overlap->getB()]->getId());
    }
}
