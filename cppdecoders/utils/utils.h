// Copyright (C) 2019 ATHENA DECODER AUTHORS; Xiangang Li; Yang Han
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ==============================================================================

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
