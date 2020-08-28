#ifndef __ATHENA_FST_H__
#define __ATHENA_FST_H__
#include <string>
#include <cstring>
#include <vector>
#include <limits>
#include <fst/types.h>
using std::vector;

namespace athena {

template <class T = float>
class FloatWeightTpl {
public:
    FloatWeightTpl() {}
    FloatWeightTpl(T f) : value_(f) {}
    FloatWeightTpl(const FloatWeightTpl<T> &w) : value_(w.value_) {}
    FloatWeightTpl<T> &operator=(const FloatWeightTpl<T> &w) {
        value_ = w.value_;
        return *this;
    }
    const T &Value() const { return value_; }
protected:
    void SetValue(const T &f) { value_ = f; }
private:
    T value_;
};


template <class T> 
class TropicalWeightTpl : public FloatWeightTpl<T> {
public:
    TropicalWeightTpl() : FloatWeightTpl<T>() {}
    TropicalWeightTpl(T f) : FloatWeightTpl<T>(f) {}
    TropicalWeightTpl(const TropicalWeightTpl<T> &w) : FloatWeightTpl<T>(w) {}

    static const TropicalWeightTpl<T> Zero() {
        return TropicalWeightTpl<T>(std::numeric_limits<T>::infinity());
    }
    static const TropicalWeightTpl<T> One() {
        return TropicalWeightTpl<T>(0.0F);
    }
    static const TropicalWeightTpl<T> NoWeight() {
        return TropicalWeightTpl<T>(std::numeric_limits<T>::quiet_NaN());
    }
};
typedef TropicalWeightTpl<float> TropicalWeight;


template <class W>
class ArcTpl {
public:
    typedef W Weight;
    typedef int Label;
    typedef int StateId;
    ArcTpl(Label i, Label o, const Weight& w, StateId s)
      : ilabel(i), olabel(o), weight(w), nextstate(s) {}
    ArcTpl() {}

    Label ilabel;
    Label olabel;
    Weight weight;
    StateId nextstate;
};
typedef ArcTpl<TropicalWeight> StdArc;


template <class A>
struct VectorState {
    typedef A Arc;
    typedef typename A::Weight Weight;
    typedef typename A::StateId StateId;
    VectorState() : final(Weight::Zero()) {}
    Weight final;              // Final weight
    vector<A> arcs;            // Arcs represenation
};
typedef VectorState<StdArc> StdVectorState;


//class ArcIterator;

class StateIterator;

template <class State>
class VectorFst{
public:
    typedef typename State::Arc Arc;
    typedef typename Arc::Weight Weight;
    typedef typename Arc::StateId StateId;
    friend class ArcIterator;
    friend class StateIterator;
    VectorFst() : start_(-1) {}
    ~VectorFst() {
        for (StateId s = 0; s < states_.size(); ++s)
            delete states_[s];
    }
    StateId Start() const { return start_; }
    Weight Final(StateId s) const { return states_[s]->final; }
    StateId NumStates() const { return states_.size(); }
    size_t NumArcs(StateId s) const { return states_[s]->arcs.size(); }
    void SetStart(StateId s) { start_ = s; }
    void SetFinal(StateId s, Weight w) { states_[s]->final = w; }
    StateId AddState() {
        states_.push_back(new State);
        return states_.size() - 1;
    }
    void AddArc(StateId s, const Arc &arc) {
        states_[s]->arcs.push_back(arc);
    }
    void DeleteStates() {
        for (StateId s = 0; s < states_.size(); ++s)
            delete states_[s];
        states_.clear();
        SetStart(-1);
    }
    State *GetState(StateId s) { return states_[s]; }
    const State *GetState(StateId s) const { return states_[s]; }
    void SetState(StateId s, State *state) { states_[s] = state; }
private:
    std::vector<State *> states_;      // States represenation.
    StateId start_;               // initial state
};
typedef VectorFst<StdVectorState> StdVectorFst;



class ArcIterator{
public:
    explicit ArcIterator(const StdVectorFst& fst, int s): i_(0){
        narcs = fst.states_[s]->arcs.size();
        arcs = narcs > 0 ? &fst.states_[s]->arcs[0] : 0;
    }
    bool Done(){
        return i_ < narcs;
    }
    StdArc& Value(){
        arcs[i_];
    }
    void Next(){
        i_++;
    }
private:
    StdArc *arcs;
    size_t narcs;
    size_t i_;
};

class StateIterator {
public:
    explicit StateIterator(const StdVectorFst &fst) : s_(0) {
        nstates = fst.states_.size();
    }
    bool Done() const {
        return  s_ >= nstates;
    }
    int Value() const { return s_;  }
    void Next() {
        ++s_;
    }
private:
    StdVectorState* states;
    int nstates;
    int s_;
};

}
#endif
