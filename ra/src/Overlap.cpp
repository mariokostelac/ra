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

bool Overlap::isTransitive(const Overlap* o2, const Overlap* o3) const {

    auto o1 = this;

    int a = o1->getA();
    int b = o1->getB();
    int c = o2->getA() != a ? o2->getA() : o2->getB();

    if (o2->isUsingSuffix(c) == o3->isUsingSuffix(c)) return false;
    if (o1->isUsingSuffix(a) != o2->isUsingSuffix(a)) return false;
    if (o1->isUsingSuffix(b) != o3->isUsingSuffix(b)) return false;

    if (!doubleEq(
            o2->hangingLength(a) + o3->hangingLength(c),
            o1->hangingLength(a),
            EPSILON * o1->getLength() + ALPHA)) {
        return false;
    }

    if (!doubleEq(
            o2->hangingLength(c) + o3->hangingLength(b),
            o1->hangingLength(b),
            EPSILON * o1->getLength() + ALPHA)) {
        return false;
    }

    debug("ISTRAN %d %d because of %d %d and %d %d",
        o1->getA(),
        o1->getB(),
        o2->getA(),
        o2->getB(),
        o3->getA(),
        o3->getB()
    );

    return true;
}

void updateOverlapIds(std::vector<Overlap*>& overlaps, std::vector<Read*>& reads) {

    for (const auto& overlap : overlaps) {
        overlap->setA(reads[overlap->getA()]->getId());
        overlap->setB(reads[overlap->getB()]->getId());
    }
}
