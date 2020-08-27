#ifndef KALDI_BASE_IO_FUNCS_H_
#define KALDI_BASE_IO_FUNCS_H_

// This header only contains some relatively low-level I/O functions.
// The full Kaldi I/O declarations are in ../util/kaldi-io.h
// and ../util/kaldi-table.h
// They were put in util/ in order to avoid making the Matrix library
// dependent on them.

#include <cctype>
#include <vector>
#include <string>
#include <kaldi/kaldi-types.h>
#include <kaldi/kaldi-error.h>
#include <kaldi/kaldi-utils.h>

namespace kaldi {

/// \addtogroup io_funcs_basic
/// @{


/// WriteBasicType is the name of the write function for bool, integer types,
/// and floating-point types. They all throw on error.
template<class T> void WriteBasicType(std::ostream &os, bool binary, T t);

/// ReadBasicType is the name of the read function for bool, integer types,
/// and floating-point types. They all throw on error.
template<class T> void ReadBasicType(std::istream &is, bool binary, T *t);


// Declare specialization for bool.
template<>
void WriteBasicType<bool>(std::ostream &os, bool binary, bool b);

template <>
void ReadBasicType<bool>(std::istream &is, bool binary, bool *b);

// Declare specializations for float and double.
template<>
void WriteBasicType<float>(std::ostream &os, bool binary, float f);

template<>
void WriteBasicType<double>(std::ostream &os, bool binary, double f);

template<>
void ReadBasicType<float>(std::istream &is, bool binary, float *f);

template<>
void ReadBasicType<double>(std::istream &is, bool binary, double *f);

// Define ReadBasicType that accepts an "add" parameter to add to
// the destination.  Caution: if used in Read functions, be careful
// to initialize the parameters concerned to zero in the default
// constructor.
template<class T>
inline void ReadBasicType(std::istream &is, bool binary, T *t, bool add) {
  if (!add) {
    ReadBasicType(is, binary, t);
  } else {
    T tmp = T(0);
    ReadBasicType(is, binary, &tmp);
    *t += tmp;
  }
}

/// Function for writing STL vectors of integer types.
template<class T> inline void WriteIntegerVector(std::ostream &os, bool binary,
                                                 const std::vector<T> &v);

/// Function for reading STL vector of integer types.
template<class T> inline void ReadIntegerVector(std::istream &is, bool binary,
                                                std::vector<T> *v);

/// Function for writing STL vectors of pairs of integer types.
template<class T>
inline void WriteIntegerPairVector(std::ostream &os, bool binary,
                                   const std::vector<std::pair<T, T> > &v);

/// Function for reading STL vector of pairs of integer types.
template<class T>
inline void ReadIntegerPairVector(std::istream &is, bool binary,
                                  std::vector<std::pair<T, T> > *v);

/// The WriteToken functions are for writing nonempty sequences of non-space
/// characters. They are not for general strings.
void WriteToken(std::ostream &os, bool binary, const char *token);
void WriteToken(std::ostream &os, bool binary, const std::string & token);

/// Peek consumes whitespace (if binary == false) and then returns the peek()
/// value of the stream.
int Peek(std::istream &is, bool binary);

/// ReadToken gets the next token and puts it in str (exception on failure).
void ReadToken(std::istream &is, bool binary, std::string *token);

/// PeekToken will return the first character of the next token, or -1 if end of
/// file.  It's the same as Peek(), except if the first character is '<' it will
/// skip over it and will return the next character.  It will unget the '<' so
/// the stream is where it was before you did PeekToken().
int PeekToken(std::istream &is, bool binary);

/// ExpectToken tries to read in the given token, and throws an exception
/// on failure.
void ExpectToken(std::istream &is, bool binary, const char *token);
void ExpectToken(std::istream &is, bool binary, const std::string & token);

/// ExpectPretty attempts to read the text in "token", but only in non-binary
/// mode.  Throws exception on failure.  It expects an exact match except that
/// arbitrary whitespace matches arbitrary whitespace.
void ExpectPretty(std::istream &is, bool binary, const char *token);
void ExpectPretty(std::istream &is, bool binary, const std::string & token);

/// @} end "addtogroup io_funcs_basic"


/// InitKaldiOutputStream initializes an opened stream for writing by writing an
/// optional binary header and modifying the floating-point precision; it will
/// typically not be called by users directly.
inline void InitKaldiOutputStream(std::ostream &os, bool binary);

/// InitKaldiInputStream initializes an opened stream for reading by detecting
/// the binary header and setting the "binary" value appropriately;
/// It will typically not be called by users directly.
inline bool InitKaldiInputStream(std::istream &is, bool *binary);

}  // end namespace kaldi.

#include "kaldi/io-funcs-inl.h"

#endif  // KALDI_BASE_IO_FUNCS_H_
