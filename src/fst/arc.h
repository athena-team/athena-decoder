#ifndef FST_LIB_ARC_H__
#define FST_LIB_ARC_H__

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <fst/float-weight.h>

namespace fst {

template <class W>
class ArcTpl {
 public:
  typedef W Weight;
  typedef int Label;
  typedef int StateId;

  ArcTpl(Label i, Label o, const Weight& w, StateId s)
      : ilabel(i), olabel(o), weight(w), nextstate(s) {}

  ArcTpl() {}

  static const std::string &Type(void) {
    static const std::string type =
        (Weight::Type() == "tropical") ? "standard" : Weight::Type();
    return type;
  }

  Label ilabel;
  Label olabel;
  Weight weight;
  StateId nextstate;
};

typedef ArcTpl<TropicalWeight> StdArc;
typedef ArcTpl<Log64Weight> Log64Arc;

}  

#endif 
