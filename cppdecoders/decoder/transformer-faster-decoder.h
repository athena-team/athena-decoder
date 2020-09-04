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

#ifndef TRANSFORMER_FASTER_DECODER_H_
#define TRANSFORMER_FASTER_DECODER_H_

#include <memory>
#include <cassert>
#include "fst/athena-fst.h"
#include "fst/types.h"
#include "decoder/hash-list.h"
#include "transformer/decodable-am-transformer.h"

namespace athena {

struct TransformerFasterDecoderOptions {
  BaseFloat beam;
  int32 max_active;
  int32 min_active;
  BaseFloat beam_delta;
  BaseFloat hash_ratio;
  int32 max_seq_len;
  BaseFloat graph_scale;
  TransformerFasterDecoderOptions(): beam(16.0),
                          max_active(std::numeric_limits<int32>::max()),
                          min_active(20), 
                          beam_delta(0.5),
                          hash_ratio(2.0),
                          graph_scale(1.0) { }
};

class TransformerFasterDecoder {
 public:
  typedef athena::StdArc Arc;
  typedef Arc::Label Label;
  typedef Arc::StateId StateId;
  typedef Arc::Weight Weight;

  TransformerFasterDecoder(const athena::StdVectorFst& fst, const TransformerFasterDecoderOptions &config);
  void SetOptions(const TransformerFasterDecoderOptions &config) { config_ = config; }
  ~TransformerFasterDecoder() { ClearToks(toks_.Clear()); }
  void Decode(TransformerDecodable *decodable);
  bool GetBestPath(std::vector<int>& trans);
 protected:
  void InitDecoding(TransformerDecodable* decodable);
  int32 NumFramesDecoded() const { return num_frames_decoded_; }
  bool ReachedFinal();
  bool EndDetect();
  void DealCompletedToken();
  void DecodeOneStep(TransformerDecodable* decodable);

  class Token {
   public:
    Arc arc_; 
    Token *prev_;
    int32 ref_count_;
    double cost_;
    double rescaled_cost_;
    // add for attention model
    std::shared_ptr<inference::PackedStates> ps_;
    std::vector<int> history_word;
    // indicate whether the token encounter eos in acoustic model
    bool encounter_eos;

    inline Token(const Arc &arc, BaseFloat ac_cost, Token *prev, BaseFloat graph_scale=1.0):
        arc_(arc), prev_(prev), ref_count_(1), rescaled_cost_(-1.0){
      if (prev) {
        prev->ref_count_++;
        cost_ = prev->cost_ + graph_scale*arc.weight.Value() + ac_cost;
        encounter_eos = prev->encounter_eos;
      } else {
        cost_ = graph_scale*arc.weight.Value() + ac_cost;
        encounter_eos = false;
      }
    }

    inline Token(const Arc &arc, Token *prev, BaseFloat graph_scale=1.0):
        arc_(arc), prev_(prev), ref_count_(1), rescaled_cost_(-1.0){
      if (prev) {
        prev->ref_count_++;
        cost_ = prev->cost_ + graph_scale*arc.weight.Value();
        encounter_eos = prev->encounter_eos;
      } else {
        cost_ = graph_scale*arc.weight.Value();
        encounter_eos = false;
      }
    }

    inline bool operator < (const Token &other) {
      return cost_ > other.cost_;
    }

    inline static void TokenDelete(Token *tok) {
      while (--tok->ref_count_ == 0) {
        Token *prev = tok->prev_;
        delete tok;
        if (prev == NULL) return;
        else tok = prev;
      }
      assert(tok->ref_count_ > 0);
    }
  };
  typedef HashList<StateId, Token*>::Elem Elem;
  double GetCutoff(Elem *list_head, size_t *tok_count, BaseFloat *adaptive_beam, Elem **best_elem);
  void PossiblyResizeHash(size_t num_toks);
  double ProcessEmitting(TransformerDecodable *decodable);
  void ProcessNonemitting(double cutoff);
  void ClearToks(Elem *list);
  HashList<StateId, Token*> toks_;
  const athena::StdVectorFst &fst_;
  TransformerFasterDecoderOptions config_;
  std::vector<StateId> queue_;  
  std::vector<BaseFloat> tmp_array_;  
  int32 num_frames_decoded_;
  std::vector<Token*> token_pool;
  Token* best_completed_tok;
};


} 


#endif
