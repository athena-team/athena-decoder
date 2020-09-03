// Copyright 2009-2011  Microsoft Corporation
//                 2013  Johns Hopkins University (author: Daniel Povey)
//
//  See ../../COPYING for clarification regarding multiple authors
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//   http://www.apache.org/licenses/LICENSE-2.0
// 
//  THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
//  KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
//  WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
//  MERCHANTABLITY OR NON-INFRINGEMENT.
//  See the Apache 2 License for the specific language governing permissions and
//  limitations under the License.

#ifndef SRC_DECODER_FASTER_DECODER_H_
#define SRC_DECODER_FASTER_DECODER_H_

#include <cassert>
#include "decoder/hash-list.h"
#include "decoder/decodable-itf.h"
#include "fst/athena-fst.h"

namespace athena {

struct FasterDecoderOptions {
    BaseFloat beam;
    int32 max_active;
    int32 min_active;
    BaseFloat beam_delta;
    BaseFloat hash_ratio;
    FasterDecoderOptions():beam(16.0), max_active(std::numeric_limits < int32 >::max()),
        min_active(20), beam_delta(0.5), hash_ratio(2.0) {}
};

class FasterDecoder {
 public:
     typedef athena::StdArc Arc;
     typedef Arc::Label Label;
     typedef Arc::StateId StateId;
     typedef Arc::Weight Weight;

     FasterDecoder(const athena::StdVectorFst & fst,
             const FasterDecoderOptions & config);

     void SetOptions(const FasterDecoderOptions & config) {
         config_ = config;
     } 
     ~FasterDecoder() {
         ClearToks(toks_.Clear());
     }

     void Decode(DecodableInterface * decodable);

     bool ReachedFinal();

     bool GetBestPath(std::vector < int >&trans);

     void InitDecoding();

     void AdvanceDecoding(DecodableInterface * decodable,
             int32 max_num_frames = -1);

     int32 NumFramesDecoded() const {
         return num_frames_decoded_;
     } protected:

     class Token {
         public:
             Arc arc_;
             Token *prev_;
             int32 ref_count_;
             double cost_;

             inline Token(const Arc & arc, BaseFloat ac_cost,
                     Token * prev):arc_(arc), prev_(prev),
             ref_count_(1) {
                 if (prev) {
                     prev->ref_count_++;
                     cost_ =
                         prev->cost_ + arc.weight.Value() +
                         ac_cost;
                 } else {
                     cost_ = arc.weight.Value() + ac_cost;
                 }
             }
             inline Token(const Arc & arc, Token * prev):arc_(arc),
             prev_(prev), ref_count_(1) {
                 if (prev) {
                     prev->ref_count_++;
                     cost_ =
                         prev->cost_ + arc.weight.Value();
                 } else {
                     cost_ = arc.weight.Value();
                 }
             }
             inline bool operator <(const Token & other) {
                 return cost_ > other.cost_;
             }

             inline static void TokenDelete(Token * tok) {
                 while (--tok->ref_count_ == 0) {
                     Token *prev = tok->prev_;
                     delete tok;
                     if (prev == NULL)
                         return;
                     else
                         tok = prev;
                 }
                 assert(tok->ref_count_ > 0);
             }
     };
     typedef HashList < StateId, Token * >::Elem Elem;

     double GetCutoff(Elem * list_head, size_t * tok_count,
             BaseFloat * adaptive_beam, Elem ** best_elem);

     void PossiblyResizeHash(size_t num_toks);

     double ProcessEmitting(DecodableInterface * decodable);

     void ProcessNonemitting(double cutoff);

     HashList < StateId, Token * >toks_;
     const athena::StdVectorFst & fst_;
     FasterDecoderOptions config_;
     std::vector < StateId > queue_;
     std::vector < BaseFloat > tmp_array_;
     int32 num_frames_decoded_;

     void ClearToks(Elem * list);
};

}  //  namespace athena

#endif  //  SRC_DECODER_FASTER_DECODER_H_ 
