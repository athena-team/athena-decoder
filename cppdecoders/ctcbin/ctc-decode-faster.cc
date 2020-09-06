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

#include <cstring>
#include "utils/graph-io.h"
#include "decoder/faster-decoder.h"
#include "ctc/decodable-am-ctc.h"
#include "utils/utils.h"
#include "gflags/gflags.h"
#include "ctc/inference.h"

#define BLOCK 1600

DEFINE_double(acoustic_scale, 3.0, "Acoustic scale for acoustic score");
DEFINE_bool(allow_partial, true, "If allow partial paths exists");
DEFINE_double(beam, 3.0, "Decoding beam");
DEFINE_int32(max_active, 200, "Max active states for one state");
DEFINE_int32(min_active, 0, "Min active states for one state");
DEFINE_int32(blank_id, 0, "Id of blank in CTC");
DEFINE_double(minus_blank, 3.0, "Minus blank value for blank id");
DEFINE_bool(ctc_prune, false, "If prune for score of blank id");
DEFINE_double(ctc_threshold, 3.0, "Threshold of CTC prune");

int main(int argc, char **argv) {
    const char *usage =
        "Faster decoder which decode utterances using CTC-based model.\n"
        "\n"
        "Usage: ctc-decode-faster [options] <am-config> <graph-in> <feature-scp> <words-wspecifier> \n";

    google::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 5) {
        std::cout << usage << std::endl;
        return 0;
    }

    float acoustic_scale = FLAGS_acoustic_scale;
    bool allow_partial = FLAGS_allow_partial;
    float beam = FLAGS_beam;
    int32 max_active = FLAGS_max_active;
    int32 min_active = FLAGS_min_active;
    float minus_blank = FLAGS_minus_blank;
    int blank_id = FLAGS_blank_id;
    bool ctc_prune = FLAGS_ctc_prune;
    float ctc_threshold = FLAGS_ctc_threshold;

    std::cout << "acoustic_scale: " << acoustic_scale << std::endl;
    std::cout << "allow_partial: " << allow_partial << std::endl;
    std::cout << "beam: " << beam << std::endl;
    std::cout << "max_active: " << max_active << std::endl;
    std::cout << "min_active: " << min_active << std::endl;
    std::cout << "blank_id: " << blank_id << std::endl;
    std::cout << "minus_blank: " << minus_blank << std::endl;
    std::cout << "ctc_prune: " << ctc_prune << std::endl;
    std::cout << "ctc_threshold: " << ctc_threshold << std::endl;

    std::string am_config = argv[1],
        fst_rxfilename = argv[2],
        feature_rspecifier = argv[3],
        words_wxfilename = argv[4];

    std::cout << "am_config: " << am_config << std::endl;
    std::cout << "fst_rxfilename: " << fst_rxfilename << std::endl;
    std::cout << "feature_rspecifier: " << feature_rspecifier << std::endl;
    std::cout << "words_wxfilename: " << words_wxfilename << std::endl;


    void* am_model;
    if(inference::STATUS_OK != inference::LoadModel(am_config.c_str(),am_model)){
        std::cerr<<"Load AM Failed!";
        return -1;
    }

    void* amhandler;
    if(inference::STATUS_OK != inference::CreateHandle(am_model,amhandler)){
        std::cerr<<"Create AM Handler Failed!";
        return -1;
    }

    std::ifstream feature_scp(feature_rspecifier,std::ios::in);
    if(!feature_scp.is_open()){
        std::cerr<<"open feature scp error";
        return -1;
    }
    std::ofstream trans_out(words_wxfilename,std::ios::out);
    if(!trans_out.is_open()){
        std::cerr<<"open trans out error";
        return -1;
    }

    // read graph
    std::cout<<"start to load graph"<<std::endl;
    athena::StdVectorFst * pgraph = ReadGraph(fst_rxfilename);
    if (pgraph == NULL) {
        std::cerr << "read graph failed";
        return -1;
    }
    std::cout<<"finish loading graph"<<std::endl;

    athena::FasterDecoderOptions options;
    options.beam = beam;
    options.max_active = max_active;
    options.min_active = min_active;

    athena::FasterDecoder decoder(*pgraph, options);
    decoder.InitDecoding();
    std::vector<int> trans;
    std::string key, path;
    while(!feature_scp.eof()){
        std::cout<<"dealing key: "<<key<<std::endl;
        if(feature_scp>>key>>path){
            short* pcm_samples = NULL;
            int short_size = 0;
            ReadPCMFile(path.c_str(), &pcm_samples, &short_size);
            inference::Input in;
            DecodableCTC decodable(amhandler, acoustic_scale, blank_id, minus_blank, ctc_prune,
                    ctc_threshold);
            bool head = true;
            while(short_size > 0){
                in.pcm_raw = (char*) pcm_samples;
                int block_size = short_size > 1.2*BLOCK ? BLOCK : short_size;
                short_size -= block_size;
                pcm_samples += block_size;
                in.pcm_size = 2*block_size;
                if(head){
                    in.first = true;
                    head = false;
                }else{
                    in.first = false;
                }
                if(short_size<=0){
                    in.last = true;
                }else{
                    in.last = false;
                }
                if(-1 == decodable.GetEncoderOutput(&in)){// calculate ctc scores
                    std::cerr<<"calculate ctc scores failed"<<std::endl;
                    return -1;
                }
                decoder.AdvanceDecoding(&decodable);
                if(in.last){
                    trans.clear();
                    decoder.GetBestPath(trans);
                    WriteTrans(trans_out, key, trans);
                    decoder.InitDecoding();
                }
            }
        }

    }

    if(inference::STATUS_OK != inference::FreeHandle(amhandler)){
        std::cerr<<"Destroy am handler failed!";
        return -1;
    }
    if(inference::STATUS_OK != inference::FreeModel(am_model)){
        std::cerr<<"Unload am model Failed!!";
        return -1;
    }

    if (pgraph) delete pgraph;
    return 0;
}
