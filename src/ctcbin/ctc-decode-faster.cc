#include <misc/graph-io.h>
#include <decoder/faster-decoder.h>
#include <kaldi/parse-options.h>
#include <ctc/decodable-am-ctc.h>
#include <misc/utils.h>
#include <cstring>

#define SAMPLE_RATE 16000

int main(int argc,char* argv[]){

    const char *usage =
        "Faster decoder which decode features using CTC-based model.\n"
        "Viterbi decoding, Only produces linear sequence; any lattice\n"
        "produced is linear\n"
        "\n"
        "Usage: ctc-decode-faster [options] <am-config> <graph-in> <feature-scp> <words-wspecifier> [prior-file]\n";


    kaldi::ParseOptions po(usage);

    float acoustic_scale=3.0;
    bool allow_partial=true;
    float beam = 15.0;
    int32 max_active = std::numeric_limits<int32>::max();
    int32 min_active = 20;
    float minus_blank=0;
    int blank_id=1;
    bool ctc_prune=false;
    float ctc_threshold=0.1;

    float* prior_log_scores = NULL;
    float prior_scale = 1.0;

    po.Register("acoustic-scale", &acoustic_scale, "Scaling factor for acoustic likelihoods");
    po.Register("allow-partial", &allow_partial, "Produce output even when final state was not reached");
    po.Register("beam", &beam, "Decoding beam.  Larger->slower, more accurate.");
    po.Register("max-active", &max_active, "Decoder max active states.  Larger->slower; " "more accurate");
    po.Register("min-active", &min_active, "Decoder min active states (don't prune if #active less than this).");
    po.Register("blank-id", &blank_id, "id of ctc <blk> label,should be >=1 ");
    po.Register("minus-blank", &minus_blank, "minus value for ctc <blk> label,should be >=0");
    po.Register("ctc-prune", &ctc_prune, "if execute ctc prune for blank frame");
    po.Register("ctc-threshold", &ctc_threshold, "threshold for ctc prune");
    po.Register("prior-scale", &prior_scale, "scale for prior scores");

    po.Read(argc,argv);


    if(po.NumArgs()!=4 && po.NumArgs()!=5){
        po.PrintUsage();
        exit(1);
    }

    std::string am_config = po.GetArg(1),
        fst_rxfilename = po.GetArg(2),
        feature_rspecifier = po.GetArg(3),
        words_wxfilename = po.GetArg(4),
        prior_rxfilename = po.GetOptArg(5);


    KALDI_LOG<<"acoustic_scale: "<<acoustic_scale;
    KALDI_LOG<<"allow_partial: "<<allow_partial;
    KALDI_LOG<<"beam: "<<beam;
    KALDI_LOG<<"max_active: "<<max_active;
    KALDI_LOG<<"min_active: "<<min_active;
    KALDI_LOG<<"blank_id: "<<blank_id;
    KALDI_LOG<<"minus_blank: "<<minus_blank;
    KALDI_LOG<<"ctc_prune: "<<ctc_prune;
    KALDI_LOG<<"ctc_threshold: "<<ctc_threshold;
    KALDI_LOG<<"prior_scale: "<<prior_scale;

    // read graph
    fst::StdVectorFst* pgraph=ReadGraph(fst_rxfilename);
    if(pgraph==NULL){
        KALDI_ERR<<"read graph failed";
        return -1;
    }

    kaldi::FasterDecoderOptions options;
    options.beam=beam;
    options.max_active=max_active;
    options.min_active=min_active;

    kaldi::FasterDecoder decoder(*pgraph,options);

    DecodableMatrixScaled decodable(acoustic_scale,blank_id,minus_blank,ctc_prune,
            ctc_threshold,prior_scale,prior_log_scores);

    decoder.Decode(&decodable);
    std::vector<int> trans;
    decoder.GetBestPath(trans);

    if(pgraph) delete pgraph;
    return 0;
}




