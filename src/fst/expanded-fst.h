#ifndef FST_LIB_EXPANDED_FST_H__
#define FST_LIB_EXPANDED_FST_H__

#include <sys/types.h>
#include <string>

#include <fst/fst.h>


namespace fst {

// A generic FST plus state count.
template <class A>
class ExpandedFst : public Fst<A> {
 public:
  typedef A Arc;
  typedef typename A::StateId StateId;

  virtual StateId NumStates() const = 0;  // State count
  virtual ExpandedFst<A> *Copy(bool safe = false) const = 0;
};


namespace internal {

//  ExpandedFst<A> case - abstract methods.
template <class A> inline
typename A::Weight Final(const ExpandedFst<A> &fst, typename A::StateId s) {
  return fst.Final(s);
}

template <class A> inline
ssize_t NumArcs(const ExpandedFst<A> &fst, typename A::StateId s) {
  return fst.NumArcs(s);
}

template <class A> inline
ssize_t NumInputEpsilons(const ExpandedFst<A> &fst, typename A::StateId s) {
  return fst.NumInputEpsilons(s);
}

template <class A> inline
ssize_t NumOutputEpsilons(const ExpandedFst<A> &fst, typename A::StateId s) {
  return fst.NumOutputEpsilons(s);
}

}  // namespace internal


// A useful alias when using StdArc.
typedef ExpandedFst<StdArc> StdExpandedFst;


// This is a helper class template useful for attaching an ExpandedFst
// interface to its implementation, handling reference counting. It
// delegates to ImplToFst the handling of the Fst interface methods.
template < class I, class F = ExpandedFst<typename I::Arc> >
class ImplToExpandedFst : public ImplToFst<I, F> {
 public:
  typedef typename I::Arc Arc;
  typedef typename Arc::Weight Weight;
  typedef typename Arc::StateId StateId;

  using ImplToFst<I, F>::GetImpl;

  virtual StateId NumStates() const { return GetImpl()->NumStates(); }

 protected:
  ImplToExpandedFst() : ImplToFst<I, F>() {}

  ImplToExpandedFst(I *impl) : ImplToFst<I, F>(impl) {}

  ImplToExpandedFst(const ImplToExpandedFst<I, F> &fst)
      : ImplToFst<I, F>(fst) {}

  ImplToExpandedFst(const ImplToExpandedFst<I, F> &fst, bool safe)
      : ImplToFst<I, F>(fst, safe) {}

};

// Function to return the number of states in an FST, counting them
// if necessary.
template <class Arc>
typename Arc::StateId CountStates(const Fst<Arc> &fst) {
  if (fst.Properties(kExpanded, false)) {
    const ExpandedFst<Arc> *efst = static_cast<const ExpandedFst<Arc> *>(&fst);
    return efst->NumStates();
  } else {
    typename Arc::StateId nstates = 0;
    for (StateIterator< Fst<Arc> > siter(fst); !siter.Done(); siter.Next())
      ++nstates;
    return nstates;
  }
}

}  // namespace fst

#endif  // FST_LIB_EXPANDED_FST_H__
