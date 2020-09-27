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

#ifndef ATTENTION_BEAMSEARCH_H
#define ATTENTION_BEAMSEARCH_H

#include "base.h"
#include "inference.h"
#include "tfmodel.h"
#include "FeatureExtraction.h"
#include <memory>

using namespace inference;

class Ctc {
public:
    Ctc();
    ~Ctc();
    int push_states(Input* in);
    int init(const char* config);
    int model_run(const TFModel* tfmodel);
    int get_new_states(encoder_output* output);
    TFModel* tfmodel;
    
private:
    // in
    DATA_TYPE<float> _input_seq;
    DATA_TYPE<float> _input_rnn_0;
    DATA_TYPE<float> _input_rnn_1;
    DATA_TYPE<float> _input_rnn_2;
    DATA_TYPE<float> _input_rnn_3;
    DATA_TYPE<float> _input_rnn_4;

    // out 
    DATA_TYPE<float> _output_state_0;
    DATA_TYPE<float> _output_state_1;
    DATA_TYPE<float> _output_state_2;
    DATA_TYPE<float> _output_state_3;
    DATA_TYPE<float> _output_state_4;
    DATA_TYPE<float> _logits;

    float* buffer_state_0;
    float* buffer_state_1;
    float* buffer_state_2;
    float* buffer_state_3;
    float* buffer_state_4;
     
    // seconds
    int pack_duration_;
    // padding left frames
    int left_context_; 
    // padding right frames
    int right_context_;
    /*
     * number of byte for one frame copy, used for online,
     * frame length 25ms, frame shift 10ms, the copy is 15ms
     */
    int repeat_size_;   
    char *repeat_audio_;
    // the size that is less than a frame-shift bytes, 16k is 320, 8k is 160, default 16k
    int remain_size_;
    char *remain_audio_;
    // frame shift byte size, 16k is 320, default is 16k
    int frame_shift_size_;
    int sample_rate_; 
    // feat size of frame after left and right padding, num_feat_dim_ = left_context_ + 1 + right_context_
    int num_feat_dim_;
    // padding zero to the first packet and last packet
    int packet_padding_;
    // reserve number frames of history feature
    int last_packet_left_;
    float *prefix_buff_;
    // the size is : left_context_ + right_context_ + 1 
    float *one_cal_feat_buff_;

    TFIO* tfio;

    FeatHandler *handler_;

    int feat_dim_;
    
};

#endif //end ATTENTION_BEAMSEARCH_H
