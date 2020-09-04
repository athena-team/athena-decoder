// Copyright (C) 2019 ATHENA DECODER AUTHORS; Xiangang Li; Yang Han; Long Yuan
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

/*================================================================*/
/* This is the main source file for Feature Extraction            */
/* Module Name: FeatureExtraction.h                               */
/* Modified date: Auguest 1, 2014                                 */
/* Written by: Weiwei Cui                                         */
/* (CopyRight)  Corporation                                  */
/*================================================================*/
#ifndef _FEATURE_EXTRACTION_H1_
#define _FEATURE_EXTRACTION_H1_

#include "CFrontend.h"
#include <sstream>

template<typename Type>
Type stringToValue(std::string s)
{
    std::stringstream ss;
    Type res = 0;
    ss<<s;
    ss>>res;
    return res;
}

template<typename T>
const std::string ValueToString(T value) {
    std::stringstream ss;
    ss<<value;
    string res;
    ss>>res;
    return res;
}

class FeatHandler {
private:
	short xxxSampleSize;
    int nMAXWaveDataNum;
    int nMaxTimeSpan;
    int nFrameSize;
    int nMaxOutputSize;
    int nContextLeft;
    int nContextRight;

    feature_conf feat_conf;
	Frontend *pFrontend;
    
    short *shortPInputSpeech;
    short *shortPInputSpeech_tmp;
    float *pFeat;
    float *pTargetFeat;
    float *pre_Feat;
    float *mean_arr_;
    float *var_arr_;
public:
	FeatHandler();
	~FeatHandler();
    
    int init(const char *conf);
    int read_cmvn_params(const char* cmvn_file);
    void cmvn_feat(float* feat_buff, int feat_len);
    int get_feat_size();
    int get_max_output_size();
    int feat_extract(const char *inputBuff, int inputSize, char *outputBuff, int &outputSize);
};



#endif
