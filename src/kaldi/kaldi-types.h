#ifndef KALDI_BASE_KALDI_TYPES_H_
#define KALDI_BASE_KALDI_TYPES_H_ 1

namespace kaldi {
// TYPEDEFS ..................................................................
#if (KALDI_DOUBLEPRECISION != 0)
typedef double  BaseFloat;
#else
typedef float   BaseFloat;
#endif
}

#ifdef _MSC_VER
#include <basetsd.h>
#define ssize_t SSIZE_T
#endif

// we can do this a different way if some platform
// we find in the future lacks stdint.h
#include <stdint.h>
#include <fst/types.h>

namespace kaldi {
  using ::int16;
  using ::int32;
  using ::int64;
  using ::uint16;
  using ::uint32;
  using ::uint64;
  typedef float   float32;
  typedef double double64;
}  // end namespace kaldi

#endif  // KALDI_BASE_KALDI_TYPES_H_
