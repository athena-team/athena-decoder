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

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "FeatureExtraction.h"

using namespace std;


FeatHandler::FeatHandler(): nMAXWaveDataNum(0),
	nFrameSize(0), nMaxOutputSize(0), nContextLeft(0), nContextRight(0) {
        nMaxTimeSpan = 30; // 3 seconds for max
	pFrontend = NULL;
	shortPInputSpeech = NULL;
	shortPInputSpeech_tmp = NULL;
	pFeat = NULL;
	pTargetFeat = NULL;
	
}

FeatHandler::~FeatHandler() {
    if (NULL != pFrontend) {
        delete pFrontend;
        pFrontend = NULL;
    }
	if (NULL != shortPInputSpeech) {
        delete [] shortPInputSpeech;
        shortPInputSpeech = NULL;
    } 
    if (NULL != shortPInputSpeech_tmp) {
        delete [] shortPInputSpeech_tmp;
        shortPInputSpeech_tmp = NULL;
    } 
    if (NULL != pFeat) {
        delete [] pFeat;
        pFeat = NULL;
    } 
    if(NULL != mean_arr_){
        delete [] mean_arr_;
        mean_arr_ = NULL;
    }

    if(NULL != var_arr_){
        delete [] var_arr_;
        var_arr_ = NULL;
    }
}

int FeatHandler::init(const char *conf) {
    //---------------- Read Config ----------------------------------------
    pFrontend = new Frontend();
    if (NULL == pFrontend){
        cerr<<"new operator fail for pFrontend"<<endl;
        return -1;
    }
    if ( -1 == pFrontend->Init( conf ) ) {
        cerr<<"["<<__FILE__<<":"<<__LINE__<<"] failed to init."<<endl;
        return -1;
    }

    feat_conf = *pFrontend->GetFeatConf();
    int nFrameDim = pFrontend->GetFeatureDim();
    xxxSampleSize = nFrameDim * sizeof(float);

    int nSampleRate = pFrontend->GetSampleRate();
    nMAXWaveDataNum = nMaxTimeSpan * nSampleRate + 200;
    shortPInputSpeech = new short[nMAXWaveDataNum];
    shortPInputSpeech_tmp = new short[nMAXWaveDataNum];
    int nMaxFrame = nMaxTimeSpan * 100;
    pFeat = new float[nMaxFrame * nFrameDim];
    if(NULL == shortPInputSpeech || NULL == shortPInputSpeech_tmp || NULL == pFeat){
        cerr<<"new operator fail for shortPInputSpeech shortPInputSpeech_tmp pFeat"<<endl;
        return -1;
    }
    
    nFrameSize = nFrameDim * sizeof(float);

    //liwei
    pFrontend->SetBuff(shortPInputSpeech, nMaxFrame);

    nContextLeft = feat_conf.context_left;
    nContextRight = feat_conf.context_right;

	nMaxOutputSize = (nMaxFrame + nContextLeft + nContextRight) * nFrameSize;

    return 0;
}

int FeatHandler::get_feat_size() {
    return xxxSampleSize;
}

int FeatHandler::feat_extract(const char *inputBuff, int inputSize, char *outputBuff, int &outputSize) 
{
    pFrontend->reset(); 

    if(inputBuff == NULL || outputBuff == NULL) {
	    cerr<<"["<<__FILE__<<":"<<__LINE__<<"] ERROR: input and output buffer should not be NULL"<<endl;
	    return -1;
    }
    
    int nWavLen;
    int nFrameDim = pFrontend->GetFeatureDim();
    nWavLen = inputSize;
    if(nWavLen <= 0) 
    {
        cerr<<"["<<__FILE__<<":"<<__LINE__<<"] ERROR: empty file"<<endl;
	return -1;
    }

    int nWaveDataNum = nWavLen / sizeof(short);	
    /*if( nWaveDataNum > nMAXWaveDataNum ) {
	cerr<<"["<<__FILE__<<":"<<__LINE__<<"] WARNING: too long pcm file"<<endl;
	return -2;
    }*/
    
    memcpy((char*)shortPInputSpeech, inputBuff, nWavLen);
    int xxxSamples = 0;
    int packageNum = 1;
    
    if ( feat_conf.feat_type == PLP_ONLINE ) 
    {
        short *pInputSpeechPtr = shortPInputSpeech;
	while ( feat_conf.package_size*packageNum < nWavLen ) 
        {
            pFrontend->extract_feature( pInputSpeechPtr, feat_conf.package_size, pFeat, &xxxSamples,0 );
	    pInputSpeechPtr += feat_conf.package_size/2;
	    packageNum ++;
	}
	pFrontend->extract_feature(pInputSpeechPtr, nWavLen - feat_conf.package_size*(packageNum-1), pFeat, &xxxSamples, 1 );
    } 
    else 
    {
	pFrontend->extract_feature(shortPInputSpeech, nWavLen, pFeat, &xxxSamples, 1 );
    }
	// print the result
    if ( xxxSamples > 0 ) {
#ifdef __OUT_FEAT_INTO_FILE__
        FILE *pfOut = fopen("./Feat.log", "ab+");
        fprintf(pfOut,"%d ", xxxSamples);
        fprintf(pfOut," %d ", nFrameSize);
        for(int i = 0; i < xxxSamples; i++)
        {
            for(int j = 0; j < nFrameSize / sizeof(float); j++) {
                fprintf(pfOut,"%f ", pFeat[i*(nFrameSize / sizeof(float))+j]);
            }
            fprintf(pfOut,"\n");
        }
        fprintf(pfOut, "======\n");
        //fclose(pfOut);
#endif
        int totalSamples = xxxSamples;
        float *ptr = pFeat;
        char *outputPtr = outputBuff;
        //cout << "xxxSamples: " << xxxSamples; 
        outputSize = nFrameSize*totalSamples;	
	// copy feature
	memcpy( (void *)outputPtr, (void *)ptr, nFrameSize*xxxSamples );

	} else {
	    cerr<<"["<<__FILE__<<":"<<__LINE__<<"] ERROR: not enough samples"<<endl;
	    pFrontend->reset(); 
	    //return -1;
	    return 0;
	}
 
	return 0;
}

int FeatHandler::read_cmvn_params(const char* cmvn_file) {
    mean_arr_ = new float[40];
    var_arr_ = new float[40];
    FILE *fp = fopen(cmvn_file, "r");
    if (fp == NULL) {
        cout << "Fail to read cmvn file" << endl;
        return -1;
    } else {
	char m[1024];
        char v[1024];
        int index = 0;
        while (-1 != fscanf(fp, "%s %s", m, v)) {
            if (0 == std::strcmp("", m) || 0 == std::strcmp("", v)) {
                continue;
            }
            mean_arr_[index] = std::atof(m);
            var_arr_[index] = std::sqrt(std::atof(v));
            index++;
        }
    }
    fclose(fp);
    return 0;
}

void FeatHandler::cmvn_feat(float* feat_buff, int feat_len) {
    for(int j = 0; j < feat_len; j++) {
        feat_buff[j] = (feat_buff[j] - mean_arr_[j % 40]) / var_arr_[j % 40];
    }
}

int FeatHandler::get_max_output_size() {
    return nMaxOutputSize; 
}

