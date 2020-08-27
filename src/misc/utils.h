#ifndef __UTILS_H__
#define __UTILS_H__

#include <misc/graph-io.h>
#include <kaldi/parse-options.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>


std::ostream& WriteTrans(std::ostream& strm,std::string key,std::vector<int>& trans);

int ReadPriorLogScores(std::string prior_rxfilename, float** prior_log_scores);

bool read_w_table(const char* wtablefile, std::vector<std::string>& table);

std::string& trim(std::string& s);
#endif
