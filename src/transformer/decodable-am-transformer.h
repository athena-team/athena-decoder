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

#ifndef __TRANSFORMER_DECODABLE__
#define __TRANSFORMER_DECODABLE__

#include <cstdlib>
#include <cstring>
#include <vector>
#include <memory>
#include "inference.h"

class TransformerDecodable{
    public:
        TransformerDecodable(void* inf_handle,float scale,int sos,int eos): 
            inf_handle_(inf_handle),
            scale_(scale), 
            sos_(sos),
            eos_(eos),
            num_frames_calculated_(0){ 
                encoder_output_ = std::shared_ptr<inference::encoder_output>(new inference::encoder_output);
            }

        float GetScale(){
            return scale_;
        }
        std::shared_ptr<inference::PackedStates> GetInitialPackedStates(){
            std::shared_ptr<inference::PackedStates> ps(inference::GetInitialPackedStates(inf_handle_));
            return ps;
        }
        int GetEncoderOutput(inference::Input* in){
            inference::GetEncoderOutput(inf_handle_,in,encoder_output_.get());
            return 0; 
        }
        int InferenceOneStep(std::vector<std::vector<int> >& batch_history_labels,
                std::shared_ptr<inference::PackedStates> current_packed_states,
                std::vector<std::vector<float> >& batch_log_scores,
                std::shared_ptr<inference::PackedStates> packed_states){
            inference::InferenceOneStep(inf_handle_,encoder_output_.get(),
                    batch_history_labels,
                    current_packed_states.get(),
                    batch_log_scores,
                    packed_states.get());

            return 0;
        }

        int sos_;
        int eos_;
    private:
        int num_frames_calculated_;
        float scale_;
        std::shared_ptr<inference::encoder_output> encoder_output_;
        void* inf_handle_;

};

#endif
