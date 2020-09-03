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

#include <algorithm>
#include <vector>
#include <cassert>
#include "decoder/transformer-beam-search-decoder.h"

namespace athena {

TransformerBeamSearchDecoder::TransformerBeamSearchDecoder( const TransformerBeamSearchDecoderOptions &opts):
    config_(opts), num_frames_decoded_(-1) {
}

void TransformerBeamSearchDecoder::InitDecoding(TransformerDecodable* decodable) {

  Token* start_token = new Token(0.0, NULL);
  start_token->ps_ = decodable->GetInitialPackedStates();
  start_token->history_word.push_back(decodable->sos_);
  completed_token_pool.clear();
  uncompleted_token_pool.clear();
  uncompleted_token_pool.push_back(start_token);
  num_frames_decoded_ = 0;
}

void TransformerBeamSearchDecoder::Decode(TransformerDecodable *decodable) {

  InitDecoding(decodable);
  do{
      ProcessEmitting(decodable);
  }while (!EndDetect());
}
bool TransformerBeamSearchDecoder::EndDetect(){

    float min_new_score = uncompleted_token_pool[0]->cost_;
    float max_completed_score = completed_token_pool[0]->cost_;
    float min_new_score_rescaled = uncompleted_token_pool[0]->rescaled_cost_;
    float max_completed_score_rescaled = completed_token_pool[0]->rescaled_cost_;
    for(int i=0;i<completed_token_pool.size();i++){
        if(completed_token_pool[i]->cost_ > max_completed_score)
            max_completed_score = completed_token_pool[i]->cost_;
        if(completed_token_pool[i]->rescaled_cost_ > max_completed_score_rescaled)
            max_completed_score_rescaled = completed_token_pool[i]->rescaled_cost_;
    }
    for(int i=0;i<uncompleted_token_pool.size();i++){
        if(uncompleted_token_pool[i]->cost_ < min_new_score)
            min_new_score = uncompleted_token_pool[i]->cost_;
        if(uncompleted_token_pool[i]->rescaled_cost_ < min_new_score_rescaled)
            min_new_score_rescaled = uncompleted_token_pool[i]->rescaled_cost_;
    }

    if( (min_new_score > max_completed_score && 
            min_new_score_rescaled > max_completed_score_rescaled)||
            num_frames_decoded_-1 > config_.max_seq_len){
        return true;
    }else{
        return false;
    }
}

bool TransformerBeamSearchDecoder::GetBestPath(std::vector<int>& trans) {
    int idx=0;
    double r_cost = completed_token_pool[0]->rescaled_cost_;
    for(int i=1;i<completed_token_pool.size();i++){
        if(completed_token_pool[i]->rescaled_cost_ < r_cost){
            r_cost = completed_token_pool[i]->rescaled_cost_;
            idx = i;
        }
    }
    // do not assign sos and eos
    trans.assign(completed_token_pool[idx]->history_word.begin()+1,
            completed_token_pool[idx]->history_word.end()-1);
  return true;
}

double TransformerBeamSearchDecoder::ProcessEmitting(TransformerDecodable *decodable) {

  num_frames_decoded_++;
  std::vector<std::vector<float> > batch_log_scores;
  std::vector<std::vector<int> > batch_history_labels;
  std::shared_ptr<inference::PackedStates> ps(NULL); 
  for(int i=0;i<uncompleted_token_pool.size();i++){
      batch_history_labels.push_back(uncompleted_token_pool[i]->history_word);
  }
  decodable->InferenceOneStep(batch_history_labels, ps, batch_log_scores, ps);
  make_heap(completed_token_pool.begin(),
          completed_token_pool.end(),
          MaxHeapCmp());
  for(int i=0;i<uncompleted_token_pool.size();i++){
      std::vector<float> log_scores = batch_log_scores[i];
       float ac_cost = -log_scores[decodable->eos_];
       Token* tok = uncompleted_token_pool[i];
       Token *new_tok = new Token(ac_cost, tok);
       new_tok->ps_ = ps;
       new_tok->history_word = tok->history_word;
       new_tok->history_word.push_back(decodable->eos_);
       new_tok->rescaled_cost_ = new_tok->cost_/num_frames_decoded_;

       if(completed_token_pool.size() >= config_.beam){
           if(new_tok->rescaled_cost_ < completed_token_pool[0]->rescaled_cost_){
               pop_heap(completed_token_pool.begin(),
                       completed_token_pool.end(),
                       MaxHeapCmp());
               Token::TokenDelete(completed_token_pool.back());
               completed_token_pool.pop_back();
               completed_token_pool.push_back(new_tok);
               push_heap(completed_token_pool.begin(),
                       completed_token_pool.end(),
                       MaxHeapCmp());
           }else{
               Token::TokenDelete(new_tok);
           }
       }else{
               completed_token_pool.push_back(new_tok);
               push_heap(completed_token_pool.begin(),
                       completed_token_pool.end(),
                       MaxHeapCmp());
       }
  }

  tmp_array.clear();
  make_heap(tmp_array.begin(),
          tmp_array.end(),
          MaxHeapCmp());
  for(int i=0;i<uncompleted_token_pool.size();i++){
      std::vector<float> log_scores = batch_log_scores[i];
      Token* tok = uncompleted_token_pool[i];
      assert(decodable->eos_ + 1 == log_scores.size());
      for(int j=0;j<decodable->eos_;j++){
          float ac_cost = -log_scores[j];
          Token *new_tok = new Token(ac_cost, tok);
          new_tok->ps_ = ps;
          new_tok->history_word = tok->history_word;
          new_tok->history_word.push_back(j);
          new_tok->rescaled_cost_ = new_tok->cost_/num_frames_decoded_;
          if(tmp_array.size() >= config_.beam){
              if(new_tok->rescaled_cost_ < tmp_array[0]->rescaled_cost_){
                  pop_heap(tmp_array.begin(),
                          tmp_array.end(),
                          MaxHeapCmp());
                  Token::TokenDelete(tmp_array.back());
                  tmp_array.pop_back();
                  tmp_array.push_back(new_tok);
                  push_heap(tmp_array.begin(),
                          tmp_array.end(),
                          MaxHeapCmp());
              }else{
                  Token::TokenDelete(new_tok);
              }
          }else{
              tmp_array.push_back(new_tok);
              push_heap(tmp_array.begin(),tmp_array.end(),MaxHeapCmp());
          }
      }
  }
  uncompleted_token_pool.swap(tmp_array);
  tmp_array.clear();

}

} // end namespace athena
