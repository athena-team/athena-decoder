#ifndef KALDI_DECODER_FASTER_DECODER_H_
#define KALDI_DECODER_FASTER_DECODER_H_

#include "kaldi/hash-list.h"
#include "kaldi/decodable-itf.h"
#include "fst/arc.h"
#include "fst/fst.h"
#include "fst/weight.h"
#include "fst/fst-decl.h"

namespace kaldi {

struct FasterDecoderOptions {
  BaseFloat beam;
  int32 max_active;
  int32 min_active;
  BaseFloat beam_delta;
  BaseFloat hash_ratio;
  FasterDecoderOptions(): beam(16.0),
                          max_active(std::numeric_limits<int32>::max()),
                          min_active(20), 
                          beam_delta(0.5),
                          hash_ratio(2.0) { }
};

class FasterDecoder {
 public:
  typedef fst::StdArc Arc;
  typedef Arc::Label Label;
  typedef Arc::StateId StateId;
  typedef Arc::Weight Weight;

  FasterDecoder(const fst::Fst<fst::StdArc> &fst, const FasterDecoderOptions &config);

  void SetOptions(const FasterDecoderOptions &config) { config_ = config; }

  ~FasterDecoder() { ClearToks(toks_.Clear()); }

  void Decode(DecodableInterface *decodable);

  bool ReachedFinal();

  bool GetBestPath(std::vector<int>& trans);

  void InitDecoding();

  void AdvanceDecoding(DecodableInterface *decodable, int32 max_num_frames = -1);

  int32 NumFramesDecoded() const { return num_frames_decoded_; }

 protected:

  class Token {
   public:
    Arc arc_; 
    Token *prev_;
    int32 ref_count_;
    double cost_;

    inline Token(const Arc &arc, BaseFloat ac_cost, Token *prev):
        arc_(arc), prev_(prev), ref_count_(1) {
      if (prev) {
        prev->ref_count_++;
        cost_ = prev->cost_ + arc.weight.Value() + ac_cost;
      } else {
        cost_ = arc.weight.Value() + ac_cost;
      }
    }
    inline Token(const Arc &arc, Token *prev):
        arc_(arc), prev_(prev), ref_count_(1) {
      if (prev) {
        prev->ref_count_++;
        cost_ = prev->cost_ + arc.weight.Value();
      } else {
        cost_ = arc.weight.Value();
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
#ifdef KALDI_PARANOID
      KALDI_ASSERT(tok->ref_count_ > 0);
#endif
    }
  };
  typedef HashList<StateId, Token*>::Elem Elem;

  double GetCutoff(Elem *list_head, size_t *tok_count, BaseFloat *adaptive_beam, Elem **best_elem);

  void PossiblyResizeHash(size_t num_toks);

  double ProcessEmitting(DecodableInterface *decodable);

  void ProcessNonemitting(double cutoff);

  HashList<StateId, Token*> toks_;
  const fst::Fst<fst::StdArc> &fst_;
  FasterDecoderOptions config_;
  std::vector<StateId> queue_;  
  std::vector<BaseFloat> tmp_array_;  
  int32 num_frames_decoded_;

  void ClearToks(Elem *list);

};


} 


#endif
