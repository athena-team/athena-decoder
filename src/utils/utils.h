#ifndef __UTILS_H__
#define __UTILS_H__

#include <utils/graph-io.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>


std::ostream& WriteTrans(std::ostream& strm,std::string key,std::vector<int>& trans);

int ReadPriorLogScores(std::string prior_rxfilename, float** prior_log_scores);

bool read_w_table(const char* wtablefile, std::vector<std::string>& table);

std::string& trim(std::string& s);

int ReadPCMFile(const char* pcm_file, short** pcm_samples, int* pcm_sample_count);
#endif
