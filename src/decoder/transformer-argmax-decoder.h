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

#ifndef TRANSFORMER_ARGMAX_DECODER_H_
#define TRANSFORMER_ARGMAX_DECODER_H_

#include <memory>
#include <algorithm>
#include "transformer/decodable-am-transformer.h"
namespace athena {

class TransformerArgmaxDecoder{
public:
    TransformerArgmaxDecoder(int max_seq_len,int sos,int eos);
    void Decode(TransformerDecodable* decodable);
    void GetBestPath(std::vector<int>& trans);
private:
    int max_seq_len;
    void* inf_handle;
    int sos;
    int eos;
    std::vector<int> labels;
};

} 


#endif
