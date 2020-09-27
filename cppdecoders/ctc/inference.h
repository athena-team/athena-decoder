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

#ifndef INFERENCE_H
#define INFERENCE_H
#include <cstring>
#include <stdlib.h>
#include <string>
#include <vector>


namespace inference
{

struct Input{
    char* pcm_raw;
    int pcm_size;
    bool first;
    bool last;
};

struct encoder_output{
    std::vector<float> enc_out;
    std::vector<int> enc_out_shape;
    std::vector<float> enc_mask;
    std::vector<int> enc_mask_shape;
    std::vector<std::vector<float> > logits;

    encoder_output(){
        enc_out.clear();
        enc_mask.clear();
        enc_out_shape.clear();
        enc_mask_shape.clear();
        logits.clear();
    }
    void clear(){
        enc_out.clear();
        enc_mask.clear();
        enc_out_shape.clear();
        enc_mask_shape.clear();
        logits.clear();
    }

    ~encoder_output(){
        enc_out.clear();
        enc_mask.clear();
        enc_out_shape.clear();
        enc_mask_shape.clear();
        logits.clear();
    }
};

enum INFStatus {
    STATUS_OK      =  0,
    STATUS_ERROR   = -1,
    STATUS_SHORT   = -2,
};

INFStatus LoadModel(const char *conf, void* &Model_Handle);

INFStatus CreateHandle(void* Model_Handle, void* &Inf_Handle);

INFStatus GetEncoderOutput(void* Inf_Handle, Input* in, encoder_output* enc_output);

INFStatus FreeHandle(void* Inf_Handle);
INFStatus FreeModel(void* Moldel_Handle);

} // namespace inference

#endif
