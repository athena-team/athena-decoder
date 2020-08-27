#ifndef FST_LIB_WEIGHT_H__
#define FST_LIB_WEIGHT_H__

#include <cmath>
#include <cctype>
#include <iostream>
#include <sstream>
#include <fst/types.h>
namespace fst {


// A representable float near .001
const float kDelta =                   1.0F/1024.0F;

// For all a,b,c: Times(c, Plus(a,b)) = Plus(Times(c,a), Times(c, b))
const uint64 kLeftSemiring =           0x0000000000000001ULL;

// For all a,b,c: Times(Plus(a,b), c) = Plus(Times(a,c), Times(b, c))
const uint64 kRightSemiring =          0x0000000000000002ULL;

const uint64 kSemiring = kLeftSemiring | kRightSemiring;

// For all a,b: Times(a,b) = Times(b,a)
const uint64 kCommutative =       0x0000000000000004ULL;

// For all a: Plus(a, a) = a
const uint64 kIdempotent =             0x0000000000000008ULL;

// For all a,b: Plus(a,b) = a or Plus(a,b) = b
const uint64 kPath =                   0x0000000000000010ULL;


// Determines direction of division.
enum DivideType { DIVIDE_LEFT,   // left division
                  DIVIDE_RIGHT,  // right division
                  DIVIDE_ANY };  // division in a commutative semiring

// NATURAL ORDER
//
// By definition:
//                 a <= b iff a + b = a
// The natural order is a negative partial order iff the semiring is
// idempotent. It is trivially monotonic for plus. It is left
// (resp. right) monotonic for times iff the semiring is left
// (resp. right) distributive. It is a total order iff the semiring
// has the path property. See Mohri, "Semiring Framework and
// Algorithms for Shortest-Distance Problems", Journal of Automata,
// Languages and Combinatorics 7(3):321-350, 2002. We define the
// strict version of this order below.

template <class W>
class NaturalLess {
 public:
  typedef W Weight;

  NaturalLess() {
    if (!(W::Properties() & kIdempotent)) {
      //FSTERROR() << "NaturalLess: Weight type is not idempotent: " << W::Type();
        std::cerr<< "NaturalLess: Weight type is not idempotent: " << W::Type();
    }
  }

  bool operator()(const W &w1, const W &w2) const {
    return (Plus(w1, w2) == w1) && w1 != w2;
  }
};


// Power is the iterated product for arbitrary semirings such that
// Power(w, 0) is One() for the semiring, and
// Power(w, n) = Times(Power(w, n-1), w)

template <class W>
W Power(W w, size_t n) {
  W result = W::One();
  for (size_t i = 0; i < n; ++i) {
    result = Times(result, w);
  }
  return result;
}

// General weight converter - raises error.
template <class W1, class W2>
struct WeightConvert {
  W2 operator()(W1 w1) const {
    //FSTERROR() << "WeightConvert: can't convert weight from \"" << W1::Type() << "\" to \"" << W2::Type();
      std::cerr << "WeightConvert: can't convert weight from \"" << W1::Type() << "\" to \"" << W2::Type();
    return W2::NoWeight();
  }
};

// Specialized weight converter to self.
template <class W>
struct WeightConvert<W, W> {
  W operator()(W w) const { return w; }
};

}  // namespace fst

#endif  // FST_LIB_WEIGHT_H__
