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

#ifndef DECODER_H
#define DECODER_H

#include "base.h"
#include "inference.h"
#include "tfmodel.h"
#include <memory>

using namespace inference;


class Decoder {
public:
    Decoder();
    ~Decoder();
    int push_states(encoder_output* enc_output, std::vector<std::vector<int> >& history_labels);
    int init_decoder();
    int model_run(const TFModel* tfmodel);
    int get_new_states(std::vector<std::vector<float> >& batch_log_scores);
    TFModel* tfmodel;

private:
    // in
    DATA_TYPE<float> _decoder_encoder_output;
    DATA_TYPE<float> _decoder_encoder_output_mask;
    DATA_TYPE<int> _decoder_step;
    DATA_TYPE<float> _decoder_history_pred;

    TFIO* tfio;
    // out 
    DATA_TYPE<float> _out_log_scores;

};





#endif //end DECODER_H
