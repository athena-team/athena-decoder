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

#ifndef  __FRONTEND_H1_
#define  __FRONTEND_H1_

#include "CFeatureClass.h"
//#include "CPLPFrontend.h" 

class Frontend {

private:
    int m_sampleFrequency;
    int m_nFeaDim;
    FeatureType_ m_featType;
    FeatureClass * m_pFE;
    feature_conf m_conf;

public:
	//提特征模块专用参数
	int m_doneWaveSamples;            /**> wave sample number which have been processed */
	int m_undoneWaveSamples;          /**> wave sample number which have not been processed */
	int m_doneFrames;                  /**> PLP feature frame number, totally finished */
	int m_doneStaticFrames;            /**> Static PLP feature frame number which have been finished */
	int m_doneDynamicAccFrames;           /**> Dynamic PLP feature frame number which have been finished */
    int m_doneDynamicDelFrames;
	int m_doneFZeroMeanFrames;         /**> On-line CMN PLP feature frame number which have been finished */
	short* m_pcm;						 /**> maitained by ams, pcm wave data */
    float m_preEmax;

public:
    
    feature_conf* GetFeatConf() { return &m_conf; };
    int GetSampleRate() { return m_sampleFrequency;};
    int GetFeatureDim() { return m_nFeaDim;};
    Frontend (); 
    ~Frontend ();

    int Init(const char *feat_conf );
    void reset();

    int SetBuff( short* wavbuf, int featbuff_size );
    int copyFirstLastFrame(  float* pbuf, int nFrameLength, int is_last_pack );
    int add_diff( float *tmp_feature, int nFrames, int is_last_pack );
    int extract_feature( short *wav, int wlen, float *feature, int* flen, int is_last_pack);   //wlen为字节数，不是采样点数
};

#endif  //__FRONTEND_H_

