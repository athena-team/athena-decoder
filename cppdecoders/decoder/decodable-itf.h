// Copyright 2009-2011  Microsoft Corporation;  Saarland University;
//                       Mirko Hannemann;  Go Vivace Inc.;
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

#ifndef SRC_DECODER_DECODABLE_ITF_H_
#define SRC_DECODER_DECODABLE_ITF_H_

#include <fst/types.h>
#include <iostream>
namespace athena {  
class DecodableInterface {
 public:
    virtual BaseFloat LogLikelihood(int32 frame, int32 index) = 0;

    virtual bool IsLastFrame(int32 frame) const = 0;

    virtual bool IsBlankFrame(int32 frame) const {
        return false;
    }
    virtual bool IsCTCPrune() const {
        return false;
    }
    virtual int32 NumFramesReady() const {
        std:: cerr << "NumFramesReady() not implemented for this decodable type.";
        return -1;
    }
    virtual int32 NumIndices() const = 0;

    virtual ~DecodableInterface() {
    }
};
}  // namespace athena

#endif  // SRC_DECODER_DECODABLE_ITF_H_
