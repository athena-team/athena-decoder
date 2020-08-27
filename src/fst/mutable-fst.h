#ifndef FST_LIB_MUTABLE_FST_H__
#define FST_LIB_MUTABLE_FST_H__

#include <stddef.h>
#include <sys/types.h>
#include <string>
#include <vector>
using std::vector;

#include <fst/expanded-fst.h>


namespace fst {

template <class A> class MutableArcIteratorData;

// An expanded FST plus mutators (use MutableArcIterator to modify arcs).
template <class A>
class MutableFst : public ExpandedFst<A> {
 public:
  typedef A Arc;
  typedef typename A::Weight Weight;
  typedef typename A::StateId StateId;

  virtual MutableFst<A> &operator=(const Fst<A> &fst) = 0;

  MutableFst<A> &operator=(const MutableFst<A> &fst) {
    return operator=(static_cast<const Fst<A> &>(fst));
  }

  virtual void SetStart(StateId) = 0;           // Set the initial state
  virtual void SetFinal(StateId, Weight) = 0;   // Set a state's final weight
  virtual void SetProperties(uint64 props,
                             uint64 mask) = 0;  // Set property bits wrt mask

  virtual StateId AddState() = 0;               // Add a state, return its ID
  virtual void AddArc(StateId, const A &arc) = 0;   // Add an arc to state

  virtual void DeleteStates(const vector<StateId>&) = 0;  // Delete some states
  virtual void DeleteStates() = 0;              // Delete all states
  virtual void DeleteArcs(StateId, size_t n) = 0;  // Delete some arcs at state
  virtual void DeleteArcs(StateId) = 0;         // Delete all arcs at state

  virtual void ReserveStates(StateId n) { }  // Optional, best effort only.
  virtual void ReserveArcs(StateId s, size_t n) { }  // Optional, Best effort.

  virtual MutableFst<A> *Copy(bool safe = false) const = 0;

  virtual void InitMutableArcIterator(StateId s,
                                      MutableArcIteratorData<A> *) = 0;
};

// Mutable arc iterator interface, templated on the Arc definition; used
// for mutable Arc iterator specializations that are returned by
// the InitMutableArcIterator MutableFst method.
template <class A>
class MutableArcIteratorBase : public ArcIteratorBase<A> {
 public:
  typedef A Arc;

  void SetValue(const A &arc) { SetValue_(arc); }  // Set current arc's content

 private:
  virtual void SetValue_(const A &arc) = 0;
};

template <class A>
struct MutableArcIteratorData {
  MutableArcIteratorBase<A> *base;  // Specific iterator
};

// Generic mutable arc iterator, templated on the FST definition
// - a wrapper around pointer to specific one.
// Here is a typical use: \code
//   for (MutableArcIterator<StdFst> aiter(&fst, s));
//        !aiter.Done();
//         aiter.Next()) {
//     StdArc arc = aiter.Value();
//     arc.ilabel = 7;
//     aiter.SetValue(arc);
//     ...
//   } \endcode
// This version requires function calls.
template <class F>
class MutableArcIterator {
 public:
  typedef F FST;
  typedef typename F::Arc Arc;
  typedef typename Arc::StateId StateId;

  MutableArcIterator(F *fst, StateId s) {
    fst->InitMutableArcIterator(s, &data_);
  }
  ~MutableArcIterator() { delete data_.base; }

  bool Done() const { return data_.base->Done(); }
  const Arc& Value() const { return data_.base->Value(); }
  void Next() { data_.base->Next(); }
  size_t Position() const { return data_.base->Position(); }
  void Reset() { data_.base->Reset(); }
  void Seek(size_t a) { data_.base->Seek(a); }
  void SetValue(const Arc &a) { data_.base->SetValue(a); }
  uint32 Flags() const { return data_.base->Flags(); }
  void SetFlags(uint32 f, uint32 m) {
    return data_.base->SetFlags(f, m);
  }

 private:
  MutableArcIteratorData<Arc> data_;
  //DISALLOW_COPY_AND_ASSIGN(MutableArcIterator);
};


namespace internal {

//  MutableFst<A> case - abstract methods.
template <class A> inline
typename A::Weight Final(const MutableFst<A> &fst, typename A::StateId s) {
  return fst.Final(s);
}

template <class A> inline
ssize_t NumArcs(const MutableFst<A> &fst, typename A::StateId s) {
  return fst.NumArcs(s);
}

template <class A> inline
ssize_t NumInputEpsilons(const MutableFst<A> &fst, typename A::StateId s) {
  return fst.NumInputEpsilons(s);
}

template <class A> inline
ssize_t NumOutputEpsilons(const MutableFst<A> &fst, typename A::StateId s) {
  return fst.NumOutputEpsilons(s);
}

}  // namespace internal


// A useful alias when using StdArc.
typedef MutableFst<StdArc> StdMutableFst;


// This is a helper class template useful for attaching a MutableFst
// interface to its implementation, handling reference counting and
// copy-on-write.
template <class I, class F = MutableFst<typename I::Arc> >
class ImplToMutableFst : public ImplToExpandedFst<I, F> {
 public:
  typedef typename I::Arc Arc;
  typedef typename Arc::Weight Weight;
  typedef typename Arc::StateId StateId;

  using ImplToFst<I, F>::GetImpl;
  using ImplToFst<I, F>::SetImpl;

  virtual void SetStart(StateId s) {
    MutateCheck();
    GetImpl()->SetStart(s);
  }

  virtual void SetFinal(StateId s, Weight w) {
    MutateCheck();
    GetImpl()->SetFinal(s, w);
  }

  virtual void SetProperties(uint64 props, uint64 mask) {
    // Can skip mutate check if extrinsic properties don't change,
    // since it is then safe to update all (shallow) copies
    uint64 exprops = kExtrinsicProperties & mask;
    if (GetImpl()->Properties(exprops) != (props & exprops))
      MutateCheck();
    GetImpl()->SetProperties(props, mask);
  }

  virtual StateId AddState() {
    MutateCheck();
    return GetImpl()->AddState();
  }

  virtual void AddArc(StateId s, const Arc &arc) {
    MutateCheck();
    GetImpl()->AddArc(s, arc);
  }

  virtual void DeleteStates(const vector<StateId> &dstates) {
    MutateCheck();
    GetImpl()->DeleteStates(dstates);
  }

  virtual void DeleteStates() {
    MutateCheck();
    GetImpl()->DeleteStates();
  }

  virtual void DeleteArcs(StateId s, size_t n) {
    MutateCheck();
    GetImpl()->DeleteArcs(s, n);
  }

  virtual void DeleteArcs(StateId s) {
    MutateCheck();
    GetImpl()->DeleteArcs(s);
  }

  virtual void ReserveStates(StateId s) {
    MutateCheck();
    GetImpl()->ReserveStates(s);
  }

  virtual void ReserveArcs(StateId s, size_t n) {
    MutateCheck();
    GetImpl()->ReserveArcs(s, n);
  }


 protected:
  ImplToMutableFst() : ImplToExpandedFst<I, F>() {}

  ImplToMutableFst(I *impl) : ImplToExpandedFst<I, F>(impl) {}


  ImplToMutableFst(const ImplToMutableFst<I, F> &fst)
      : ImplToExpandedFst<I, F>(fst) {}

  ImplToMutableFst(const ImplToMutableFst<I, F> &fst, bool safe)
      : ImplToExpandedFst<I, F>(fst, safe) {}

  void MutateCheck() {
  }

 private:
};


}  // namespace fst

#endif  // FST_LIB_MUTABLE_FST_H__
