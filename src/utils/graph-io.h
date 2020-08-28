#ifndef __GRAPH_IO__
#define __GRAPH_IO__
#include <fstream>
#include <iostream>
#include <math.h>
#include <fst/athena-fst.h>

static const int32 kFstMagicNumber = 2125659606;

std::istream &ReadTypeIO(std::istream &strm, std::string *s); 
std::istream &ReadTypeIO(std::istream &strm, int32& number );
std::istream &ReadTypeIO(std::istream &strm, int64& number );
std::istream &ReadTypeIO(std::istream &strm, uint64& number );
std::istream &ReadTypeIO(std::istream &strm, float& number );
athena::StdVectorFst* ReadGraph(std::string filename);
void WriteGraph(athena::StdVectorFst* pfst);

#endif
