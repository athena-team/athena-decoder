#include <cstring>
#include "utils/graph-io.h"
#include "decoder/faster-decoder.h"
#include "ctc/decodable-am-ctc.h"
#include "utils/utils.h"
#include "gflags/gflags.h"
#include "ctc/inference.h"

#define SAMPLE_RATE 16000

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

    inference::Input in;
    /* fill the speech data to in struct*/
    in.pcm_raw = new char[100]; // fake speech data
    in.pcm_size = 100;// speech size
    in.first = true;
    in.last = true;

    // read graph
    athena::StdVectorFst * pgraph = ReadGraph(fst_rxfilename);
    if (pgraph == NULL) {
        std::cerr << "read graph failed";
        return -1;
    }
    std::vector<std::string> table;
    read_w_table(words_wxfilename.c_str(), table);
    athena::FasterDecoderOptions options;
    options.beam = beam;
    options.max_active = max_active;
    options.min_active = min_active;

    athena::FasterDecoder decoder(*pgraph, options);

    DecodableCTC decodable(acoustic_scale, blank_id, minus_blank, ctc_prune,
                   ctc_threshold);
    decodable.CalCTCScores(amhandler, &in);

    decoder.InitDecoding();
    decoder.AdvanceDecoding(&decodable);
    std::vector < int >trans;
    decoder.GetBestPath(trans);
    std::cout<<"transcript is : ";
    for(int i=0;i<trans.size();i++){
        std::cout<<table[trans[i]]<<" ";
    }
    std::cout<<std::endl;

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
