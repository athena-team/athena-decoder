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

#ifndef TRANSFORMER_BEAM_SEARCH_DECODER_H_
#define TRANSFORMER_BEAM_SEARCH_DECODER_H_

#include <memory>
#include <algorithm>
#include "transformer/decodable-am-transformer.h"
#include "fst/types.h"

namespace athena {

struct TransformerBeamSearchDecoderOptions {
  int32 beam;
  int32 max_seq_len;
  TransformerBeamSearchDecoderOptions(): beam(4),
                          max_seq_len(100) { }
};

class TransformerBeamSearchDecoder {
 public:

  TransformerBeamSearchDecoder(const TransformerBeamSearchDecoderOptions &config);
  void SetOptions(const TransformerBeamSearchDecoderOptions &config) { config_ = config; }
  bool EndDetect();
  void Decode(TransformerDecodable *decodable);
  bool GetBestPath(std::vector<int>& trans);
  void InitDecoding(TransformerDecodable* decodable);
  int32 NumFramesDecoded() const { return num_frames_decoded_; }
 protected:
  class Token {
   public:
    Token *prev_;
    int32 ref_count_;
    double cost_;
    double rescaled_cost_;
    std::shared_ptr<inference::PackedStates> ps_;
    std::vector<int> history_word;

    inline Token(BaseFloat ac_cost, Token *prev):
        prev_(prev), ref_count_(1), rescaled_cost_(-1.0){
      if (prev) {
        prev->ref_count_++;
        cost_ = prev->cost_ +  ac_cost;
      } else {
        cost_ = ac_cost;
      }
    }

    inline static void TokenDelete(Token *tok) {
      while (--tok->ref_count_ == 0) {
        Token *prev = tok->prev_;
        delete tok;
        if (prev == NULL) return;
        else tok = prev;
      }
    }
  };

  struct MaxHeapCmp{
      inline bool operator()(const Token* a, const Token* b){
          return a->rescaled_cost_ < b->rescaled_cost_;
      }
  };

  double ProcessEmitting(TransformerDecodable *decodable);
  TransformerBeamSearchDecoderOptions config_;
  int32 num_frames_decoded_;
  std::vector<Token*> completed_token_pool;
  std::vector<Token*> uncompleted_token_pool;
  std::vector<Token*> tmp_array;

};


} 


#endif
