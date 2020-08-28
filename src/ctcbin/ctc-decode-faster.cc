#include <utils/graph-io.h>
#include <decoder/faster-decoder.h>
#include <ctc/decodable-am-ctc.h>
#include <utils/utils.h>
#include <cstring>
#include <cxxopts.hpp>

#define SAMPLE_RATE 16000

int main(int argc,const char** argv){

    const char *usage =
        "Faster decoder which decode features using CTC-based model.\n"
        "Viterbi decoding, Only produces linear sequence; any lattice\n"
        "produced is linear\n"
        "\n"
        "Usage: ctc-decode-faster [options] <am-config> <graph-in> <feature-scp> <words-wspecifier> [prior-file]\n";

    cxxopts::Options opt_parser(argv[0],"test");

    opt_parser.add_options()
        ("acoustic-scale","Scaling ",cxxopts::value<float>()->default_value("3.0"))
        ("allow-partial","Scaling ",cxxopts::value<bool>()->default_value("true"))
        ("beam","Scaling ",cxxopts::value<float>()->default_value("15.0"))
        ("max-active","Scaling ",cxxopts::value<int32>()->default_value("200"))
        ("min-active","Scaling ",cxxopts::value<int32>()->default_value("0"))
        ("blank-id","Scaling ",cxxopts::value<int>()->default_value("1"))
        ("minus-blank","Scaling ",cxxopts::value<float>()->default_value("0"))
        ("ctc-prune","Scaling ",cxxopts::value<bool>()->default_value("false"))
        ("ctc-threshold","Scaling ",cxxopts::value<float>()->default_value("0.1"))
        ("prior-scale","Scaling ",cxxopts::value<float>()->default_value("1.0"));
    auto result = opt_parser.parse(argc,argv);

    float acoustic_scale=result["acoustic-scale"].as<float>();
    bool allow_partial=result["allow-partial"].as<bool>();
    float beam = result["beam"].as<float>();
    int32 max_active = result["max-active"].as<int32>();
    int32 min_active = result["min-active"].as<int32>();
    float minus_blank=result["minus-blank"].as<float>();
    int blank_id=result["blank-id"].as<int>();
    bool ctc_prune=result["ctc-prune"].as<bool>();
    float ctc_threshold=result["ctc-threshold"].as<float>();
    float* prior_log_scores = NULL;
    float prior_scale = result["prior-scale"].as<float>();

    std::string am_config = argv[1],
        fst_rxfilename = argv[2],
        feature_rspecifier = argv[3],
        words_wxfilename = argv[4],
        prior_rxfilename = argv[5];


    std::cout<<"acoustic_scale: "<<acoustic_scale;
    std::cout<<"allow_partial: "<<allow_partial;
    std::cout<<"beam: "<<beam;
    std::cout<<"max_active: "<<max_active;
    std::cout<<"min_active: "<<min_active;
    std::cout<<"blank_id: "<<blank_id;
    std::cout<<"minus_blank: "<<minus_blank;
    std::cout<<"ctc_prune: "<<ctc_prune;
    std::cout<<"ctc_threshold: "<<ctc_threshold;
    std::cout<<"prior_scale: "<<prior_scale;

    // read graph
    athena::StdVectorFst* pgraph=ReadGraph(fst_rxfilename);
    if(pgraph==NULL){
        std::cerr<<"read graph failed";
        return -1;
    }

    athena::FasterDecoderOptions options;
    options.beam=beam;
    options.max_active=max_active;
    options.min_active=min_active;

    athena::FasterDecoder decoder(*pgraph,options);

    DecodableMatrixScaled decodable(acoustic_scale,blank_id,minus_blank,ctc_prune,
            ctc_threshold,prior_scale,prior_log_scores);

    decoder.Decode(&decodable);
    std::vector<int> trans;
    decoder.GetBestPath(trans);

    if(pgraph) delete pgraph;
    return 0;
}




