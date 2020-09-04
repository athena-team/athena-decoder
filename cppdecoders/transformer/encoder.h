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

#ifndef ENCODER_H
#define ENCODER_H

#include "base.h"
#include "inference.h"
#include "tfmodel.h"
#include "FeatureExtraction.h"
#include <memory>

using namespace inference;

class Encoder {
public:
    Encoder();
    ~Encoder();
    int push_states(Input* in);
    int init_encoder(const char* enc_config);
    int model_run(const TFModel* tfmodel);
    int get_new_states(encoder_output* enc_output);
    TFModel* tfmodel;
    
private:
    // in
    DATA_TYPE<int> _encoder_input_len;
    DATA_TYPE<float> _encoder_input;

    // out 
    DATA_TYPE<int> _out_mask;
    DATA_TYPE<float> _out_enc;

    TFIO* tfio;

    FeatHandler *handler_;

    int feat_dim_;
    
};

#endif //end ENCODER_H
