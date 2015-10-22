/*!
 * @file Overlap.cpp
 *
 * @brief Overlap class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date May 14, 2015
 */

#include "Overlap.hpp"

const double EPSILON = 0.15;
const double ALPHA = 3;

static inline bool doubleEq(double x, double y, double eps) {
    return y <= x + eps && x <= y + eps;
}

DepotObjectType Overlap::type_ = DepotObjectType::kOverlap;

Overlap::Overlap(const Read* read_a, int32_t a_hang, const Read* read_b,
    int32_t b_hang, bool is_innie, double err_rate, double orig_err_rate)
        : read_a_(read_a), a_hang_(a_hang),
          read_b_(read_b), b_hang_(b_hang),
          is_innie_(is_innie),
          is_dovetail_(true) {

    ASSERT(read_a != nullptr, "Overlap", "Read A is nullptr!");
    ASSERT(read_b != nullptr, "Overlap", "Read B is nullptr!");

    a_lo_ = a_hang > 0 ? a_hang : 0;
    a_hi_ = read_a->length() - (b_hang < 0 ? -b_hang : 0);
    b_lo_ = a_hang < 0 ? -a_hang : 0;
    b_hi_ = read_b->length() - (b_hang > 0 ? b_hang : 0);
    a_rc_ = false;
    b_rc_ = is_innie;

    err_rate_ = err_rate;
    orig_err_rate_ = orig_err_rate;
}

Overlap::Overlap(const Read* read_a, uint32_t a_lo, uint32_t a_hi, bool a_rc,
    const Read* read_b, uint32_t b_lo, uint32_t b_hi, bool b_rc,
    double err_rate, double orig_err_rate)
        : read_a_(read_a), a_hang_(),
          read_b_(read_b), b_hang_(),
          is_innie_(a_rc != b_rc),
          is_dovetail_(false),
          a_lo_(a_lo), a_hi_(a_hi),
          b_lo_(b_lo), b_hi_(b_hi),
          a_rc_(a_rc),
          b_rc_(b_rc),
          err_rate_(err_rate),
          orig_err_rate_(orig_err_rate) {

    ASSERT(read_a != nullptr, "Overlap", "Read A is nullptr!");
    ASSERT(read_b != nullptr, "Overlap", "Read B is nullptr!");
}

double Overlap::covered_percentage(uint32_t read_id) const {

    auto a_id = read_a_->id();
    auto b_id = read_b_->id();

    assert(read_id == a_id || read_id == b_id);

    if (read_id == a_id) {
        return length(a_id) / (double) read_a_->length();
    } else if (read_id == b_id) {
        return length(b_id) / (double) read_b_->length();
    }

    return 0;
}

uint32_t Overlap::length(uint32_t read_id) const {

    auto a_id = read_a_->id();

    assert(read_id == a_id || read_id == read_b_->id());

    if (read_id == a_id) {
        return a_hi_ - a_lo_;
    }

    return b_hi_ - b_lo_;
}

uint32_t Overlap::length() const {
    return (length(read_a_->id()) + length(read_b_->id())) / 2;
}

std::string Overlap::extract_overlapped_part(uint32_t read_id) const {

    auto a_id = read_a_->id();

    assert(read_id == a_id || read_id == read_b_->id());
    assert(read_a_ != nullptr && read_b_ != nullptr);

    if (read_id == a_id) {
        // [lo, hi>
        return std::string(read_a_->sequence()).substr(a_lo_, a_hi_ - a_lo_);
    } else {
        if (is_innie_) {
            return reverseComplement(std::string(read_b_->sequence())).substr(
                b_lo_, b_hi_ - b_lo_);
        }

        return std::string(read_b_->sequence()).substr(b_lo_, b_hi_ - b_lo_);
    }
}

bool Overlap::is_using_prefix(uint32_t read_id) const {

    if (read_id == a()) {
        if (a_hang_ <= 0) return true;

    } else if (read_id == b()) {
        if (is_innie_ == false && a_hang_ >= 0) return true;
        if (is_innie_ == true && b_hang_ <= 0) return true;
    }

    return false;
}

bool Overlap::is_using_suffix(uint32_t read_id) const {

    if (read_id == a()) {
        if (b_hang_ >= 0) return true;

    } else if (read_id == b()) {
        if (is_innie_ == false && b_hang_ <= 0) return true;
        if (is_innie_ == true && a_hang_ >= 0) return true;
    }

    return false;
}

uint32_t Overlap::hanging_length(uint32_t read_id) const {

    if (read_id == a()) {
        int hanging = 0;
        if (a_hang_ > 0) {
            hanging += a_hang_;
        }
        if (b_hang_ < 0) {
            hanging += abs(b_hang_);
        }
        return hanging;

    } else if (read_id == b()) {
        int hanging = 0;
        if (a_hang_ < 0) {
            hanging += abs(a_hang_);
        }
        if (b_hang_ > 0) {
            hanging += b_hang_;
        }
        return hanging;
    }

    ASSERT(false, "Overlap", "wrong read id");
}

bool Overlap::is_transitive(const Overlap* o2, const Overlap* o3) const {

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
        o1->a(), o1->b(),
        o2->a(), o2->b(),
        o3->a(), o3->b()
    );

    return true;
}

Overlap* Overlap::clone() const {
    return new Overlap(*this);
}

void Overlap::print(std::ostream& o) const {
  o << a() << "\t" << b() << "\t" << (is_innie_ ? 'I' : 'N') << "\t";
  o << a_hang_ << "\t" << b_hang_  << "\t" << err_rate_ << "\t";
  o << orig_err_rate_ << std::endl;
}

std::string Overlap::repr() const {

    const std::string overlap("=======");

    std::string a = overlap;
    std::string b = overlap;

    char buff[256];
    if (a_hang_ > 0) {
        sprintf(buff, "%-7d", a_hang_);
        a = "=======" + a;
        b = std::string(buff) + b;
    } else if (a_hang_ < 0) {
        sprintf(buff, "%-7d", a_hang_);
        a = std::string(buff) + a;
        b = "=======" + b;
    }

    if (b_hang_ > 0) {
        sprintf(buff, "%7d", b_hang_);
        a = a + std::string(buff);
        b = b + "=======";
    } else if  (b_hang_ < 0) {
        sprintf(buff, "%7d", b_hang_);
        a = a + "=======";
        b = b + std::string(buff);
    }

    for (int i = a.size() - 1; i >= 0; i--) {
        if (a[i] == '=') {
            a[i] = '>';
            break;
        }
    }

    if (is_innie()) {
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

    return a + " " + std::to_string(this->a()) + "\n" +
           b + " " + std::to_string(this->b()) + "\n";
}

void Overlap::serialize(char** bytes, uint32_t* bytes_length) const {

}

Overlap* Overlap::deserialize(const char* bytes) {
    return nullptr;
}
