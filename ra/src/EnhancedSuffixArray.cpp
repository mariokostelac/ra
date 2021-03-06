/*!
 * @file EnhancedSuffixArray.cpp
 *
 * @brief EnhancedSuffixArray class source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Apr 18, 2015
 */

#include "EnhancedSuffixArray.hpp"

#define SENTINEL_H '~'
#define SENTINEL_L '!'

#define MAX_SIZE 2147483645U

static int getChar(int i, const unsigned char* s, int csize) {
    if (csize == sizeof(int)) return ((int*) s)[i];
    return ((unsigned char*) s)[i];
}

static void getBuckets(std::vector<int>& buckets, const unsigned char* s, int n, int csize, int end) {

    for (int i = 0; i < (int) buckets.size(); ++i) {
        buckets[i] = 0;
    }

    for (int i = 0; i < n; ++i) {
        ++buckets[getChar(i, s, csize)];
    }

    int sum = 0;
    for (int i = 0; i < (int) buckets.size(); ++i) {
        sum += buckets[i];
        buckets[i] = (end == 1) ? sum : sum - buckets[i];
    }
}

static void induceL(std::vector<int>& suftab, std::vector<int>& buckets, const unsigned char* s, int n,
    int csize, std::vector<bool>& t) {

    getBuckets(buckets, s, n, csize, 0);

    for (int i = 0; i < n; ++i) {
        int j = suftab[i] - 1;
        if (j >= 0 && !t[j]) suftab[buckets[getChar(j, s, csize)]++] = j;
    }
}

static void induceS(std::vector<int>& suftab, std::vector<int>& buckets, const unsigned char* s, int n,
    int csize, std::vector<bool>& t) {

    getBuckets(buckets, s, n, csize, 1);

    for (int i = n - 1; i >= 0; --i) {
        int j = suftab[i] - 1;
        if (j >= 0 && t[j]) suftab[--buckets[getChar(j, s, csize)]] = j;
    }
}

static bool isLMS(int i, std::vector<bool>& t) {
    return i > 0 && t[i] && !t[i - 1];
}

EnhancedSuffixArray::EnhancedSuffixArray(const std::string& str) {

    ASSERT(str.size() <= MAX_SIZE, "ESA", "invalid input string length");

    Timer timer;
    timer.start();

    str_ = str;
    str_ += SENTINEL_H;
    str_ += SENTINEL_L;

    n_ = str_.size();
    suftab_.resize(n_);

    createSuffixArray((unsigned char*) &str_[0], n_, sizeof(unsigned char));
    createLongestCommonPrefixTable();
    createChildTable();

    timer.stop();
    timer.print("ESA", "construction");
}

void EnhancedSuffixArray::intervalSubInterval(int* s, int* e, int i, int j, char c) const {

    if (i > j) {
        *s = -1;
        *e = -1;
        return;
    }

    int i1 = (i < childtab_[i] && childtab_[i] <= j) ? childtab_[i] : childtab_[j];

    if (str_[suftab_[i] + lcptab_[i1]] == c) {
        *s = i; *e = i1 - 1;
        return;
    }

    // .nextlIndex if not .down nor .up
    while (childtab_[i1] != -1 && i1 < n_ && !(lcptab_[childtab_[i1]] > lcptab_[i1] || lcptab_[i1] > lcptab_[i1 + 1])) {
        int i2 = childtab_[i1];
        if (str_[suftab_[i1] + lcptab_[i2]] == c) {
            *s = i1; *e = i2 - 1;
            return;
        }
        i1 = i2;
    }

    if (str_[suftab_[i1] + lcptab_[i1]] == c) {
        *s = i1; *e = j;
        return;
    }

    *s = -1;
    *e = -1;
}

int EnhancedSuffixArray::intervalLcpLen(int i, int j) const {
    return lcptab_[childtab_[(i < childtab_[i] && childtab_[i] <= j) ? i : j]];
}

size_t EnhancedSuffixArray::sizeInBytes() const {

    size_t bytesLen = 0;
    size_t size = sizeof(int);

    bytesLen += size; // n_
    bytesLen += n_; // str_
    bytesLen += (size_t) n_ * size; // suftab_
    bytesLen += (size_t) n_ * size; // lcptab_
    bytesLen += (size_t) n_ * size; // childtab_

    return bytesLen;
}

void EnhancedSuffixArray::serialize(char** bytes, size_t* bytesLen) const {

    *bytesLen = sizeInBytes();
    *bytes = new char[*bytesLen];

    size_t size = sizeof(int);
    size_t ptr = 0;

    std::memcpy(*bytes+ ptr, &n_, size);
    ptr += size;

    std::memcpy(*bytes + ptr, &str_[0], n_);
    ptr += n_;

    std::memcpy(*bytes + ptr, &suftab_[0], n_ * size);
    ptr += n_ * size;

    std::memcpy(*bytes + ptr, &lcptab_[0], n_ * size);
    ptr += n_ * size;

    std::memcpy(*bytes + ptr, &childtab_[0], n_ * size);
}

EnhancedSuffixArray* EnhancedSuffixArray::deserialize(const char* bytes) {

    EnhancedSuffixArray* esa = new EnhancedSuffixArray();

    size_t size = sizeof(int);
    size_t ptr = 0;

    std::memcpy(&esa->n_, bytes + ptr, size);
    ptr += size;

    esa->str_.resize(esa->n_);
    esa->suftab_.resize(esa->n_);
    esa->lcptab_.resize(esa->n_);
    esa->childtab_.resize(esa->n_);

    std::memcpy(&esa->str_[0], bytes + ptr, esa->n_);
    ptr += esa->n_;

    std::memcpy(&esa->suftab_[0], bytes + ptr, esa->n_ * size);
    ptr += esa->n_ * size;

    std::memcpy(&esa->lcptab_[0], bytes + ptr, esa->n_ * size);
    ptr += esa->n_ * size;

    std::memcpy(&esa->childtab_[0], bytes + ptr, esa->n_ * size);

    return esa;
}

void EnhancedSuffixArray::print() const {

    printf("  Idx Suftab LcpTab ChildTab Suffix\n");

    for (int i = 0; i < n_; ++i) {
        printf("%5d %6d %6d %8d %-s\n", i, suftab_[i], lcptab_[i], childtab_[i],
            str_.substr(suftab_[i], n_ - suftab_[i]).c_str());
    }
}

void EnhancedSuffixArray::createSuffixArray(const unsigned char* s, int n, int csize, int alphabetSize) {

    // S-type = true, L-type = false
    std::vector<bool> t(n);
    t[n - 1] = true;
    t[n - 2] = false;

    for (int i = n - 3; i >= 0; --i) {
        t[i] = (getChar(i, s, csize) < getChar(i + 1, s, csize) ||
            (getChar(i, s, csize) == getChar(i + 1, s, csize) && t[i + 1])) ? true : false;
    }

    std::vector<int> buckets(alphabetSize);
    getBuckets(buckets, s, n, csize, 1);

    for (int i = 0; i < n; ++i) suftab_[i] = -1;
    for (int i = 1; i < n; ++i) {
        if (isLMS(i, t)) suftab_[--buckets[getChar(i, s, csize)]] = i;
    }

    induceL(suftab_, buckets, s, n, csize, t);
    induceS(suftab_, buckets, s, n, csize, t);

    std::vector<int>().swap(buckets);

    int n1 = 0;
    for (int i = 0; i < n; ++i) {
        if (isLMS(suftab_[i], t)) suftab_[n1++] = suftab_[i];
    }

    for (int i = n1; i < n; ++i) suftab_[i] = -1;

    int name = 0, prev = -1;
    for (int i = 0; i < n1; ++i) {

        int pos = suftab_[i];
        bool diff = false;

        for (int d = 0; d < n; ++d) {
            if (prev == -1 || getChar(pos + d, s, csize) != getChar(prev + d, s, csize) ||
                t[pos + d] != t[prev + d]) {

                diff = true;
                break;

            } else if (d > 0 && (isLMS(pos + d, t) || isLMS(prev + d, t))) {
                break;
            }
        }

        if (diff) {
            ++name;
            prev = pos;
        }

        pos = (pos % 2 == 0) ? pos / 2 : (pos - 1) / 2;
        suftab_[n1 + pos] = name - 1;
    }

    for (int i = n - 1, j = n - 1; i >= n1; --i) {
        if (suftab_[i] >= 0) suftab_[j--] = suftab_[i];
    }

    int* s1 = &suftab_[n - n1];

    if (name < n1) {
        createSuffixArray((unsigned char*) s1, n1, sizeof(int), name);
    } else {
        for (int i = 0; i < n1; ++i) suftab_[s1[i]] = i;
    }

    buckets.resize(alphabetSize);

    for (int i = 1, j = 0; i < n; ++i) {
        if (isLMS(i, t)) s1[j++] = i;
    }

    getBuckets(buckets, s, n, csize, 1);

    for (int i = 0; i < n1; ++i) suftab_[i] = s1[suftab_[i]];
    for (int i = n1; i < n; ++i) suftab_[i] = -1;

    for (int i = n1 - 1; i >= 0; --i) {
        int j = suftab_[i];
        suftab_[i] = -1;
        suftab_[--buckets[getChar(j, s, csize)]] = j;
    }

    induceL(suftab_, buckets, s, n, csize, t);
    induceS(suftab_, buckets, s, n, csize, t);
}

void EnhancedSuffixArray::createLongestCommonPrefixTable() {

    lcptab_.resize(n_, 0);

    std::vector<int> rank(n_);
    for (int i = 0; i < n_; ++i) rank[suftab_[i]] = i;

    int h = 0;

    for (int i = 0; i < n_; ++i) {
        if (rank[i] > 0) {
            int j = suftab_[rank[i] - 1];

            while (str_[i + h] == str_[j + h]) ++h;

            lcptab_[rank[i]] = h;
            if (h > 0) --h;
        }
    }
}

void EnhancedSuffixArray::createChildTable() {

    childtab_.resize(n_, -1);

    // childTable = .up + .down + .nextlIndex (which can be stored in 4B)
    // 1. Construction of .up and .down
    std::stack<int> st;
    int lastIndex = -1;

    st.push(0);
    for (int i = 1; i < n_; ++i) {
        while (lcptab_[i] < lcptab_[st.top()]) {
            lastIndex = st.top();
            st.pop();
            if ((lcptab_[i] <= lcptab_[st.top()]) && (lcptab_[st.top()] != lcptab_[lastIndex])) {
                // .down
                childtab_[st.top()] = lastIndex;
            }
        }

        if (lastIndex != -1) {
            // .up
            childtab_[i - 1] = lastIndex;
            lastIndex = -1;
        }

        st.push(i);
    }

    // 2. Construciton of .nextlIndex
    std::stack<int>().swap(st);

    st.push(0);
    for (int i = 1; i < n_; ++i) {
        while (lcptab_[i] < lcptab_[st.top()]) st.pop();

        if (lcptab_[i] == lcptab_[st.top()]) {
            // .nextlIndex
            childtab_[st.top()] = i;
            st.pop();
        }

        st.push(i);
    }
}
