#include <utils/graph-io.h>
#include <decoder/faster-decoder.h>
#include <ctc/decodable-am-ctc.h>
#include <utils/utils.h>
#include <cstring>
//#include <cxxopts.hpp>
#include <gflags/gflags.h>

#define SAMPLE_RATE 16000


DEFINE_double(acoustic_scale,3.0,"acoustic scale for am model");
DEFINE_bool(allow_partial,true,"acoustic scale for am model");
DEFINE_double(beam,3.0,"acoustic scale for am model");
DEFINE_int32(max_active,200,"acoustic scale for am model");
DEFINE_int32(min_active,0,"acoustic scale for am model");
DEFINE_int32(blank_id,0,"acoustic scale for am model");
DEFINE_double(minus_blank,3.0,"acoustic scale for am model");
DEFINE_bool(ctc_prune,false,"acoustic scale for am model");
DEFINE_double(ctc_threshold,3.0,"acoustic scale for am model");

int main(int argc,char** argv){

    const char *usage =
        "Faster decoder which decode utterances using CTC-based model.\n"
        "\n"
        "Usage: ctc-decode-faster [options] <am-config> <graph-in> <feature-scp> <words-wspecifier> \n";

    google::ParseCommandLineFlags(&argc,&argv,true);

    if(argc != 5){
        std::cout<<usage<<std::endl;
        return 0;
    }

    float acoustic_scale=FLAGS_acoustic_scale;
    bool allow_partial=FLAGS_allow_partial;
    float beam = FLAGS_beam;
    int32 max_active = FLAGS_max_active;
    int32 min_active = FLAGS_min_active;
    float minus_blank = FLAGS_minus_blank;
    int blank_id = FLAGS_blank_id; 
    bool ctc_prune= FLAGS_ctc_prune;
    float ctc_threshold = FLAGS_ctc_threshold;

    std::cout<<"acoustic_scale: "<<acoustic_scale<<std::endl;
    std::cout<<"allow_partial: "<<allow_partial<<std::endl;
    std::cout<<"beam: "<<beam<<std::endl;
    std::cout<<"max_active: "<<max_active<<std::endl;
    std::cout<<"min_active: "<<min_active<<std::endl;
    std::cout<<"blank_id: "<<blank_id<<std::endl;
    std::cout<<"minus_blank: "<<minus_blank<<std::endl;
    std::cout<<"ctc_prune: "<<ctc_prune<<std::endl;
    std::cout<<"ctc_threshold: "<<ctc_threshold<<std::endl;



    std::string am_config = argv[1],
        fst_rxfilename = argv[2],
        feature_rspecifier = argv[3],
        words_wxfilename = argv[4];

    std::cout<<"am_config: "<<am_config<<std::endl;
    std::cout<<"fst_rxfilename: "<<fst_rxfilename<<std::endl;
    std::cout<<"feature_rspecifier: "<<feature_rspecifier<<std::endl;
    std::cout<<"words_wxfilename: "<<words_wxfilename<<std::endl;



    // read graph
    athena::StdVectorFst* pgraph=ReadGraph(fst_rxfilename);
    if(pgraph==NULL){
        std::cerr<<"read graph failed";
        return -1;
    }
    WriteGraph(pgraph);
    return 0;


    athena::FasterDecoderOptions options;
    options.beam=beam;
    options.max_active=max_active;
    options.min_active=min_active;

    athena::FasterDecoder decoder(*pgraph,options);

    DecodableMatrixScaled decodable(acoustic_scale,blank_id,minus_blank,ctc_prune,
            ctc_threshold);

    decoder.Decode(&decodable);
    std::vector<int> trans;
    decoder.GetBestPath(trans);

    if(pgraph) delete pgraph;
    return 0;
}




