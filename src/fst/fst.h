#ifndef FST_LIB_FST_H__
#define FST_LIB_FST_H__

#include <stddef.h>
#include <sys/types.h>
#include <cmath>
#include <string>

#include <fst/types.h>
#include <fst/arc.h>
#include <fst/properties.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <fst/fst-decl.h>

namespace fst {

bool IsFstHeader(std::istream &, const std::string &);

class FstHeader;
template <class A> class StateIteratorData;
template <class A> class ArcIteratorData;
template <class A> class MatcherBase;

class FstHeader {
 public:
  enum {
    HAS_ISYMBOLS = 0x1,          // Has input symbol table
    HAS_OSYMBOLS = 0x2,          // Has output symbol table
    IS_ALIGNED   = 0x4,          // Memory-aligned (where appropriate)
  } Flags;

  FstHeader() : version_(0), flags_(0), properties_(0), start_(-1),
                numstates_(0), numarcs_(0) {}
  const std::string &FstType() const { return fsttype_; }
  const std::string &ArcType() const { return arctype_; }
  int32 Version() const { return version_; }
  int32 GetFlags() const { return flags_; }
  uint64 Properties() const { return properties_; }
  int64 Start() const { return start_; }
  int64 NumStates() const { return numstates_; }
  int64 NumArcs() const { return numarcs_; }

  void SetFstType(const std::string& type) { fsttype_ = type; }
  void SetArcType(const std::string& type) { arctype_ = type; }
  void SetVersion(int32 version) { version_ = version; }
  void SetFlags(int32 flags) { flags_ = flags; }
  void SetProperties(uint64 properties) { properties_ = properties; }
  void SetStart(int64 start) { start_ = start; }
  void SetNumStates(int64 numstates) { numstates_ = numstates; }
  void SetNumArcs(int64 numarcs) { numarcs_ = numarcs; }

 private:

  std::string fsttype_;                   // E.g. "vector"
  std::string arctype_;                   // E.g. "standard"
  int32 version_;                    // Type version #
  int32 flags_;                      // File format bits
  uint64 properties_;                // FST property bits
  int64 start_;                      // Start state
  int64 numstates_;                  // # of states
  int64 numarcs_;                    // # of arcs
};


// Specifies matcher action.
enum MatchType { MATCH_INPUT,      // Match input label.
                 MATCH_OUTPUT,     // Match output label.
                 MATCH_BOTH,       // Match input or output label.
                 MATCH_NONE,       // Match nothing.
                 MATCH_UNKNOWN };  // Match type unknown.

//
// Fst INTERFACE CLASS DEFINITION
//

// A generic FST, templated on the arc definition, with
// common-demoninator methods (use StateIterator and ArcIterator to
// iterate over its states and arcs).
template <class A>
class Fst {
 public:
  typedef A Arc;
  typedef typename A::Weight Weight;
  typedef typename A::StateId StateId;

  virtual ~Fst() {}

  virtual StateId Start() const = 0;          // Initial state

  virtual Weight Final(StateId) const = 0;    // State's final weight

  virtual size_t NumArcs(StateId) const = 0;  // State's arc count

  virtual size_t NumInputEpsilons(StateId)
      const = 0;                              // State's input epsilon count

  virtual size_t NumOutputEpsilons(StateId)
      const = 0;                              // State's output epsilon count

  // If test=false, return stored properties bits for mask (some poss. unknown)
  // If test=true, return property bits for mask (computing o.w. unknown)
  virtual uint64 Properties(uint64 mask, bool test)
      const = 0;  // Property bits

  virtual const std::string& Type() const = 0;    // Fst type name

  // Get a copy of this Fst. The copying behaves as follows:
  //
  // (1) The copying is constant time if safe = false or if safe = true
  // and is on an otherwise unaccessed Fst.
  //
  // (2) If safe = true, the copy is thread-safe in that the original
  // and copy can be safely accessed (but not necessarily mutated) by
  // separate threads. For some Fst types, 'Copy(true)' should only be
  // called on an Fst that has not otherwise been accessed. Its behavior
  // is undefined otherwise.
  //
  // (3) If a MutableFst is copied and then mutated, then the original is
  // unmodified and vice versa (often by a copy-on-write on the initial
  // mutation, which may not be constant time).
  virtual Fst<A> *Copy(bool safe = false) const = 0;


  // For generic state iterator construction; not normally called
  // directly by users.
  virtual void InitStateIterator(StateIteratorData<A> *) const = 0;

  // For generic arc iterator construction; not normally called
  // directly by users.
  virtual void InitArcIterator(StateId s, ArcIteratorData<A> *) const = 0;

  // For generic matcher construction; not normally called
  // directly by users.
  virtual MatcherBase<A> *InitMatcher(MatchType match_type) const;

 protected:
};


//
// STATE and ARC ITERATOR DEFINITIONS
//

// State iterator interface templated on the Arc definition; used
// for StateIterator specializations returned by the InitStateIterator
// Fst method.
template <class A>
class StateIteratorBase {
 public:
  typedef A Arc;
  typedef typename A::StateId StateId;

  virtual ~StateIteratorBase() {}

  bool Done() const { return Done_(); }       // End of iterator?
  StateId Value() const { return Value_(); }  // Current state (when !Done)
  void Next() { Next_(); }      // Advance to next state (when !Done)
  void Reset() { Reset_(); }    // Return to initial condition

 private:
  // This allows base class virtual access to non-virtual derived-
  // class members of the same name. It makes the derived class more
  // efficient to use but unsafe to further derive.
  virtual bool Done_() const = 0;
  virtual StateId Value_() const = 0;
  virtual void Next_() = 0;
  virtual void Reset_() = 0;
};


// StateIterator initialization data

template <class A> struct StateIteratorData {
  StateIteratorBase<A> *base;   // Specialized iterator if non-zero
  typename A::StateId nstates;  // O.w. total # of states
};


template <class F>
class StateIterator {
 public:
  typedef F FST;
  typedef typename F::Arc Arc;
  typedef typename Arc::StateId StateId;

  explicit StateIterator(const F &fst) : s_(0) {
    fst.InitStateIterator(&data_);
  }

  ~StateIterator() { if (data_.base) delete data_.base; }

  bool Done() const {
    return data_.base ? data_.base->Done() : s_ >= data_.nstates;
  }

  StateId Value() const { return data_.base ? data_.base->Value() : s_; }

  void Next() {
    if (data_.base)
      data_.base->Next();
    else
      ++s_;
  }

  void Reset() {
    if (data_.base)
      data_.base->Reset();
    else
      s_ = 0;
  }

 private:
  StateIteratorData<Arc> data_;
  StateId s_;

  //DISALLOW_COPY_AND_ASSIGN(StateIterator);
};


// Flags to control the behavior on an arc iterator:
static const uint32 kArcILabelValue    = 0x0001;  // Value() gives valid ilabel
static const uint32 kArcOLabelValue    = 0x0002;  //  "       "     "    olabel
static const uint32 kArcWeightValue    = 0x0004;  //  "       "     "    weight
static const uint32 kArcNextStateValue = 0x0008;  //  "       "     " nextstate
static const uint32 kArcNoCache   = 0x0010;       // No need to cache arcs

static const uint32 kArcValueFlags =
                  kArcILabelValue | kArcOLabelValue |
                  kArcWeightValue | kArcNextStateValue;

static const uint32 kArcFlags = kArcValueFlags | kArcNoCache;


// Arc iterator interface, templated on the Arc definition; used
// for Arc iterator specializations that are returned by the InitArcIterator
// Fst method.
template <class A>
class ArcIteratorBase {
 public:
  typedef A Arc;
  typedef typename A::StateId StateId;

  virtual ~ArcIteratorBase() {}

  bool Done() const { return Done_(); }            // End of iterator?
  const A& Value() const { return Value_(); }      // Current arc (when !Done)
  void Next() { Next_(); }           // Advance to next arc (when !Done)
  size_t Position() const { return Position_(); }  // Return current position
  void Reset() { Reset_(); }         // Return to initial condition
  void Seek(size_t a) { Seek_(a); }  // Random arc access by position
  uint32 Flags() const { return Flags_(); }  // Return current behavorial flags
  void SetFlags(uint32 flags, uint32 mask) {  // Set behavorial flags
    SetFlags_(flags, mask);
  }

 private:
  // This allows base class virtual access to non-virtual derived-
  // class members of the same name. It makes the derived class more
  // efficient to use but unsafe to further derive.
  virtual bool Done_() const = 0;
  virtual const A& Value_() const = 0;
  virtual void Next_() = 0;
  virtual size_t Position_() const = 0;
  virtual void Reset_() = 0;
  virtual void Seek_(size_t a) = 0;
  virtual uint32 Flags_() const = 0;
  virtual void SetFlags_(uint32 flags, uint32 mask) = 0;
};


// ArcIterator initialization data
template <class A> struct ArcIteratorData {
  ArcIteratorBase<A> *base;  // Specialized iterator if non-zero
  const A *arcs;             // O.w. arcs pointer
  size_t narcs;              // ... and arc count
  int *ref_count;            // ... and reference count if non-zero
};


template <class F>
class ArcIterator {
   public:
  typedef F FST;
  typedef typename F::Arc Arc;
  typedef typename Arc::StateId StateId;

  ArcIterator(const F &fst, StateId s) : i_(0) {
    fst.InitArcIterator(s, &data_);
  }

  explicit ArcIterator(const ArcIteratorData<Arc> &data) : data_(data), i_(0) {
    if (data_.ref_count)
      ++(*data_.ref_count);
  }

  ~ArcIterator() {
    if (data_.base)
      delete data_.base;
    else if (data_.ref_count)
      --(*data_.ref_count);
  }

  bool Done() const {
    return data_.base ?  data_.base->Done() : i_ >= data_.narcs;
  }

  const Arc& Value() const {
    return data_.base ? data_.base->Value() : data_.arcs[i_];
  }

  void Next() {
    if (data_.base)
      data_.base->Next();
    else
      ++i_;
  }

  void Reset() {
    if (data_.base)
      data_.base->Reset();
    else
      i_ = 0;
  }

  void Seek(size_t a) {
    if (data_.base)
      data_.base->Seek(a);
    else
      i_ = a;
  }

  size_t Position() const {
    return data_.base ? data_.base->Position() : i_;
  }

  uint32 Flags() const {
    if (data_.base)
      return data_.base->Flags();
    else
      return kArcValueFlags;
  }

  void SetFlags(uint32 flags, uint32 mask) {
    if (data_.base)
      data_.base->SetFlags(flags, mask);
  }

 private:
  ArcIteratorData<Arc> data_;
  size_t i_;
  //DISALLOW_COPY_AND_ASSIGN(ArcIterator);
};

//
// MATCHER DEFINITIONS
//

template <class A>
MatcherBase<A> *Fst<A>::InitMatcher(MatchType match_type) const {
  return 0;  // Use the default matcher
}


//
// FST ACCESSORS - Useful functions in high-performance cases.
//

namespace internal {

// General case - requires non-abstract, 'final' methods. Use for inlining.
template <class F> inline
typename F::Arc::Weight Final(const F &fst, typename F::Arc::StateId s) {
  return fst.F::Final(s);
}

template <class F> inline
ssize_t NumArcs(const F &fst, typename F::Arc::StateId s) {
  return fst.F::NumArcs(s);
}

template <class F> inline
ssize_t NumInputEpsilons(const F &fst, typename F::Arc::StateId s) {
  return fst.F::NumInputEpsilons(s);
}

template <class F> inline
ssize_t NumOutputEpsilons(const F &fst, typename F::Arc::StateId s) {
  return fst.F::NumOutputEpsilons(s);
}


//  Fst<A> case - abstract methods.
template <class A> inline
typename A::Weight Final(const Fst<A> &fst, typename A::StateId s) {
  return fst.Final(s);
}

template <class A> inline
ssize_t NumArcs(const Fst<A> &fst, typename A::StateId s) {
  return fst.NumArcs(s);
}

template <class A> inline
ssize_t NumInputEpsilons(const Fst<A> &fst, typename A::StateId s) {
  return fst.NumInputEpsilons(s);
}

template <class A> inline
ssize_t NumOutputEpsilons(const Fst<A> &fst, typename A::StateId s) {
  return fst.NumOutputEpsilons(s);
}

}  // namespace internal

// A useful alias when using StdArc.
typedef Fst<StdArc> StdFst;


//
//  CONSTANT DEFINITIONS
//

const int kNoStateId   =  -1;  // Not a valid state ID
const int kNoLabel     =  -1;  // Not a valid label

//
// Fst IMPLEMENTATION BASE
//
// This is the recommended Fst implementation base class. It will
// handle reference counts, property bits, type information and symbols.
//

template <class A> class FstImpl {
 public:
  typedef typename A::Weight Weight;
  typedef typename A::StateId StateId;

  FstImpl() : properties_(0), type_("null"){}

  FstImpl(const FstImpl<A> &impl) : properties_(impl.properties_), type_(impl.type_){}

  virtual ~FstImpl() {
  }

  const std::string& Type() const { return type_; }

  void SetType(const std::string &type) { type_ = type; }

  virtual uint64 Properties() const { return properties_; }

  virtual uint64 Properties(uint64 mask) const { return properties_ & mask; }

  void SetProperties(uint64 props) {
    properties_ &= kError;          // kError can't be cleared
    properties_ |= props;
  }

  void SetProperties(uint64 props, uint64 mask) {
    properties_ &= ~mask | kError;  // kError can't be cleared
    properties_ |= props & mask;
  }

  // Allows (only) setting error bit on const FST impls
  void SetProperties(uint64 props, uint64 mask) const {
    if (mask != kError)
      std::cerr << "FstImpl::SetProperties() const: can only set kError";
    properties_ |= kError;
  }

 protected:
  mutable uint64 properties_;           // Property bits

 private:
  std::string type_;                 // Unique name of Fst class
  void operator=(const FstImpl<A> &impl);  // disallow
};


// This is a helper class template useful for attaching an Fst interface to
// its implementation, handling reference counting.
template < class I, class F = Fst<typename I::Arc> >
class ImplToFst : public F {
 public:
  typedef typename I::Arc Arc;
  typedef typename Arc::Weight Weight;
  typedef typename Arc::StateId StateId;

  virtual ~ImplToFst() { 
  }

  virtual StateId Start() const { return impl_->Start(); }

  virtual Weight Final(StateId s) const { return impl_->Final(s); }

  virtual size_t NumArcs(StateId s) const { return impl_->NumArcs(s); }

  virtual size_t NumInputEpsilons(StateId s) const {
    return impl_->NumInputEpsilons(s);
  }

  virtual size_t NumOutputEpsilons(StateId s) const {
    return impl_->NumOutputEpsilons(s);
  }

  virtual uint64 Properties(uint64 mask, bool test) const {
  }

  virtual const std::string& Type() const { return impl_->Type(); }

 protected:
  ImplToFst() : impl_(0) {}

  ImplToFst(I *impl) : impl_(impl) {}

  ImplToFst(const ImplToFst<I, F> &fst) {
    impl_ = fst.impl_;
  }

  ImplToFst(const ImplToFst<I, F> &fst, bool safe) {
    if (safe) {
      impl_ = new I(*(fst.impl_));
    } else {
      impl_ = fst.impl_;
      impl_->IncrRefCount();
    }
  }

  I *GetImpl() const { return impl_; }

  void SetImpl(I *impl, bool own_impl = true) {
    impl_ = impl;
  }

 private:
  I *impl_;
};

};  // namespace fst

#endif  // FST_LIB_FST_H__
