#ifndef __GRAPH_IO__
#define __GRAPH_IO__
#include <fstream>
#include <iostream>
#include <kaldi/kaldi-types.h>
#include <fst/mutable-fst.h>
#include <fst/vector-fst.h>
#include <fst/fst-decl.h>
#include <math.h>
#include <kaldi/kaldi-error.h>
//#include <kaldi/kaldi-lattice.h>

static const int32 kFstMagicNumber = 2125659606;
typedef int Label;
typedef int StateId;


std::istream &ReadTypeIO(std::istream &strm, std::string *s); 
std::istream &ReadTypeIO(std::istream &strm, int32& number );
std::istream &ReadTypeIO(std::istream &strm, int64& number );
std::istream &ReadTypeIO(std::istream &strm, uint64& number );
std::istream &ReadTypeIO(std::istream &strm, float& number );
fst::StdVectorFst* ReadGraph(std::string filename);
void WriteGraph(fst::StdVectorFst* pfst);

/*
void PrintCompactLatticeWeight(std::ostream& strm,const kaldi::CompactLatticeWeight& weight);
void PrintCompactLatticeState(std::ostream& strm,kaldi::CompactLattice& lat,kaldi::CompactLatticeArc::StateId s);
bool WriteCompactLattice(std::ostream& strm,std::string key,kaldi::CompactLattice& lat);
void ApplyProbabilityScale(float scale, fst::StdMutableFst *fst); 
*/

#endif
