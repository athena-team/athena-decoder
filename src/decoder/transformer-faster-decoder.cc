#include <fst/types.h>
#include <cassert>
#include <algorithm>
#include <vector>
#include "decoder/transformer-faster-decoder.h"

namespace athena {


TransformerFasterDecoder::TransformerFasterDecoder(const athena::StdVectorFst& fst,
                             const TransformerFasterDecoderOptions &opts):
    fst_(fst), config_(opts), num_frames_decoded_(-1) {
  assert(config_.hash_ratio >= 1.0);  // less doesn't make much sense.
  assert(config_.max_active > 1);
  assert(config_.min_active >= 0 && config_.min_active <= config_.max_active);
  toks_.SetSize(1000);  // just so on the first frame we do something reasonable.
}


void TransformerFasterDecoder::InitDecoding(TransformerDecodable* decodable) {
  // clean up from last time:
  ClearToks(toks_.Clear());
  StateId start_state = fst_.Start();
  assert(start_state != -1);
  Arc dummy_arc(0, 0, Weight::One(), start_state);
  Token* start_token = new Token(dummy_arc, NULL,config_.graph_scale);
  start_token->ps_ = decodable->GetInitialPackedStates();
  start_token->history_word.push_back(decodable->sos_);
  toks_.Insert(start_state, start_token);
  ProcessNonemitting(std::numeric_limits<float>::max());
  num_frames_decoded_ = 0;
  best_completed_tok = NULL;
  token_pool.clear();
}


void TransformerFasterDecoder::Decode(TransformerDecodable *decodable) {

  InitDecoding(decodable);
  while (!EndDetect()) {
      DecodeOneStep(decodable);
  }
}
bool TransformerFasterDecoder::EndDetect(){

    const Elem *e = toks_.GetList();  
    if(e == NULL || num_frames_decoded_-1 > config_.max_seq_len){
        int idx = token_pool.size()-1;
        if(idx < 0){
            return true;
        } 
        best_completed_tok = token_pool[idx];
        for(int i=idx-1;i>=0;i--){ //reverse loop means prefer longer transcripts
            if(best_completed_tok->rescaled_cost_ > token_pool[i]->rescaled_cost_){ 
                Token::TokenDelete(best_completed_tok);
                best_completed_tok = token_pool[i];
            }else{
                Token::TokenDelete(token_pool[i]);
            }
        }
        return true;
    }else{
        return false;
    }
}
void TransformerFasterDecoder::DecodeOneStep(TransformerDecodable* decodable){
    double weight_cutoff = ProcessEmitting(decodable);
    ProcessNonemitting(weight_cutoff);
    DealCompletedToken();
}

void TransformerFasterDecoder::DealCompletedToken(){
    for (Elem *e = const_cast<Elem*>(toks_.GetList()), *e_tail; e != NULL;  e = e_tail){
        StateId state = e->key;
        Token *tok = e->val;
        e_tail = e->tail;
        if(tok->encounter_eos && fst_.Final(e->key).Value() != Weight::Zero().Value()){
            tok->rescaled_cost_ = (tok->cost_ + fst_.Final(e->key).Value())/num_frames_decoded_;
            tok->ref_count_++;
            token_pool.push_back(tok);
        }
    }
}

bool TransformerFasterDecoder::ReachedFinal() {
  for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail) {
    if (e->val->cost_ != std::numeric_limits<double>::infinity() &&
        fst_.Final(e->key).Value() != Weight::Zero().Value())
      return true;
  }
  return false;
}

bool TransformerFasterDecoder::GetBestPath(std::vector<int>& trans) {

  if (best_completed_tok == NULL){
      std::cerr<<"do not get best completed tok";
      return false;
  } 
  trans.clear();
  for (Token *tok = best_completed_tok; tok != NULL; tok = tok->prev_) {
      if(tok->arc_.olabel != 0){
          trans.push_back(tok->arc_.olabel);
      }
  }
  std::reverse(trans.begin(), trans.end());
  Token::TokenDelete(best_completed_tok);
  return true;
}


double TransformerFasterDecoder::GetCutoff(Elem *list_head, size_t *tok_count,
                                BaseFloat *adaptive_beam, Elem **best_elem) {
  double best_cost = std::numeric_limits<double>::infinity();
  size_t count = 0;
  if (config_.max_active == std::numeric_limits<int32>::max() &&
      config_.min_active == 0) {
    for (Elem *e = list_head; e != NULL; e = e->tail, count++) {
      double w = e->val->cost_;
      if (w < best_cost) {
        best_cost = w;
        if (best_elem) *best_elem = e;
      }
    }
    if (tok_count != NULL) *tok_count = count;
    if (adaptive_beam != NULL) *adaptive_beam = config_.beam;
    return best_cost + config_.beam;
  } else {
    tmp_array_.clear();
    for (Elem *e = list_head; e != NULL; e = e->tail, count++) {
      double w = e->val->cost_;
      tmp_array_.push_back(w);
      if (w < best_cost) {
        best_cost = w;
        if (best_elem) *best_elem = e;
      }
    }
    if (tok_count != NULL) *tok_count = count;
    double beam_cutoff = best_cost + config_.beam,
        min_active_cutoff = std::numeric_limits<double>::infinity(),
        max_active_cutoff = std::numeric_limits<double>::infinity();
    
    if (tmp_array_.size() > static_cast<size_t>(config_.max_active)) {
      std::nth_element(tmp_array_.begin(),
                       tmp_array_.begin() + config_.max_active,
                       tmp_array_.end());
      max_active_cutoff = tmp_array_[config_.max_active];
    }
    if (max_active_cutoff < beam_cutoff) { // max_active is tighter than beam.
      if (adaptive_beam)
        *adaptive_beam = max_active_cutoff - best_cost + config_.beam_delta;
      return max_active_cutoff;
    }    
    if (tmp_array_.size() > static_cast<size_t>(config_.min_active)) {
      if (config_.min_active == 0) min_active_cutoff = best_cost;
      else {
        std::nth_element(tmp_array_.begin(),
                         tmp_array_.begin() + config_.min_active,
                         tmp_array_.size() > static_cast<size_t>(config_.max_active) ?
                         tmp_array_.begin() + config_.max_active :
                         tmp_array_.end());
        min_active_cutoff = tmp_array_[config_.min_active];
      }
    }
    if (min_active_cutoff > beam_cutoff) { // min_active is looser than beam.
      if (adaptive_beam)
        *adaptive_beam = min_active_cutoff - best_cost + config_.beam_delta;
      return min_active_cutoff;
    } else {
      *adaptive_beam = config_.beam;
      return beam_cutoff;
    }
  }
}

void TransformerFasterDecoder::PossiblyResizeHash(size_t num_toks) {
  size_t new_sz = static_cast<size_t>(static_cast<BaseFloat>(num_toks)
                                      * config_.hash_ratio);
  if (new_sz > toks_.Size()) {
    toks_.SetSize(new_sz);
  }
}

// ProcessEmitting returns the likelihood cutoff used.
double TransformerFasterDecoder::ProcessEmitting(TransformerDecodable *decodable) {
  int32 frame = num_frames_decoded_;
  Elem *last_toks = toks_.Clear();
  size_t tok_cnt;
  BaseFloat adaptive_beam;
  Elem *best_elem = NULL;
  double weight_cutoff = GetCutoff(last_toks, &tok_cnt,
                                   &adaptive_beam, &best_elem);
  PossiblyResizeHash(tok_cnt);  // This makes sure the hash is always big enough.
  double next_weight_cutoff = std::numeric_limits<double>::infinity();

  if (best_elem) {
    StateId state = best_elem->key;
    Token *tok = best_elem->val;
    std::vector<std::vector<float> > batch_log_scores;
    std::vector<std::vector<int> > batch_history_labels;
    batch_history_labels.push_back(tok->history_word);
    std::shared_ptr<inference::PackedStates> ps(NULL);
    decodable->InferenceOneStep(batch_history_labels,
            tok->ps_, batch_log_scores, ps);
    for (athena::ArcIterator aiter(fst_, state);
         !aiter.Done();
         aiter.Next()) {
      const Arc &arc = aiter.Value();
      if (arc.ilabel != 0) {  // we'd propagate..
          BaseFloat ac_cost = -batch_log_scores[0][arc.ilabel-1];
          ac_cost*=decodable->GetScale();
          double new_weight = arc.weight.Value() + tok->cost_ + ac_cost;
          if (new_weight + adaptive_beam < next_weight_cutoff)
              next_weight_cutoff = new_weight + adaptive_beam;
      }
    }
  }

  std::vector<std::vector<float> > batch_log_scores;
  std::vector<std::vector<int> > batch_history_labels;
  std::shared_ptr<inference::PackedStates> ps(NULL); 
  for (Elem *e = last_toks, *e_tail; e != NULL; e = e_tail) {  // loop this way
    StateId state = e->key;
    Token *tok = e->val;
    if (tok->cost_ < weight_cutoff && tok->encounter_eos==false) {  // not pruned.
        batch_history_labels.push_back(tok->history_word);
    }
    e_tail = e->tail;
  }
  if(batch_history_labels.size()!=0){
      decodable->InferenceOneStep(batch_history_labels, ps, batch_log_scores, ps);
  }

  int cnt = 0;
  for (Elem *e = last_toks, *e_tail; e != NULL; e = e_tail) {  // loop this way
    // because we delete "e" as we go.
    StateId state = e->key;
    Token *tok = e->val;
    if (tok->cost_ < weight_cutoff && tok->encounter_eos==false) {  // not pruned.
       vector<float> log_scores = batch_log_scores[cnt];
       cnt++;
       // when 'eos' is the best label this step,
       // the token reach end and form a completed token
       std::vector<float>::iterator biggest = std::max_element(log_scores.begin(), log_scores.end());
       int biggest_idx = std::distance(std::begin(log_scores), biggest);
       if(biggest_idx == decodable->eos_ || num_frames_decoded_ > config_.max_seq_len){
           Token* new_tok = new Token(tok->arc_,tok->prev_,config_.graph_scale);
           new_tok->ps_ = tok->ps_;
           new_tok->history_word = tok->history_word;
           new_tok->cost_ = tok->cost_ + (-log_scores[decodable->eos_]*decodable->GetScale());
           new_tok->encounter_eos = true;
           toks_.Insert(state, new_tok);
           e_tail = e->tail;
           toks_.Delete(e);
           continue;
       } 

      assert(state == tok->arc_.nextstate);
      for (athena::ArcIterator aiter(fst_, state);
           !aiter.Done();
           aiter.Next()) {
        Arc arc = aiter.Value();
        if (arc.ilabel != 0) {  // propagate..
          BaseFloat ac_cost = -log_scores[arc.ilabel-1];
          ac_cost*=decodable->GetScale();

          double new_weight = arc.weight.Value() + tok->cost_ + ac_cost;
          if (new_weight < next_weight_cutoff) {  // not pruned..
            Token *new_tok = new Token(arc, ac_cost, tok, config_.graph_scale);

            new_tok->ps_ = ps;
            new_tok->history_word = tok->history_word;
            new_tok->history_word.push_back(arc.ilabel-1);

            Elem *e_found = toks_.Find(arc.nextstate);
            if (new_weight + adaptive_beam < next_weight_cutoff)
                next_weight_cutoff = new_weight + adaptive_beam;
            if (e_found == NULL) {
                toks_.Insert(arc.nextstate, new_tok);
            } else {
                if ( *(e_found->val) < *new_tok ) {
                    Token::TokenDelete(e_found->val);
                    e_found->val = new_tok;
                } else {
                    Token::TokenDelete(new_tok);
                }
            }
          }
        }
      }
    }
    e_tail = e->tail;
    Token::TokenDelete(e->val);
    toks_.Delete(e);
  }
  num_frames_decoded_++;
  return next_weight_cutoff;
}

// TODO: first time we go through this, could avoid using the queue.
void TransformerFasterDecoder::ProcessNonemitting(double cutoff) {
    // Processes nonemitting arcs for one frame. 

    assert(queue_.empty());
    for (const Elem *e = toks_.GetList(); e != NULL;  e = e->tail)
        queue_.push_back(e->key);
    while (!queue_.empty()) {
        StateId state = queue_.back();
        queue_.pop_back();
        Token *tok = toks_.Find(state)->val;  // would segfault if state not
        if (tok->cost_ > cutoff) { // Don't bother processing successors.
            continue;
        }
        assert(tok != NULL && state == tok->arc_.nextstate);
        for (athena::ArcIterator aiter(fst_, state);
                !aiter.Done();
                aiter.Next()) {
            const Arc &arc = aiter.Value();
            if (arc.ilabel == 0) {  // propagate nonemitting only...
                Token *new_tok = new Token(arc, tok, config_.graph_scale);
                new_tok->history_word = tok->history_word;
                new_tok->ps_ = tok->ps_;
                if (new_tok->cost_ > cutoff) {  // prune
                    Token::TokenDelete(new_tok);
                } else {
                    Elem *e_found = toks_.Find(arc.nextstate);
                    if (e_found == NULL) {
                        toks_.Insert(arc.nextstate, new_tok);
                        queue_.push_back(arc.nextstate);
                    } else {
                        if ( *(e_found->val) < *new_tok ) {
                            Token::TokenDelete(e_found->val);
                            e_found->val = new_tok;
                            queue_.push_back(arc.nextstate);
                        } else {
                            Token::TokenDelete(new_tok);
                        }
                    }
                }
            }
        }
    }
}

void TransformerFasterDecoder::ClearToks(Elem *list) {
    for (Elem *e = list, *e_tail; e != NULL; e = e_tail) {
        Token::TokenDelete(e->val);
        e_tail = e->tail;
        toks_.Delete(e);
    }
}

} // end namespace athena.
