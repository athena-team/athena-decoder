#include <cstring>
#include "utils/graph-io.h"
#include "decoder/transformer-faster-decoder.h"
#include "transformer/decodable-am-transformer.h"
#include "utils/utils.h"
#include "transformer/inference.h"
#include "gflags/gflags.h"


DEFINE_double(acoustic_scale, 3.0, "Acoustic scale for acoustic score");
DEFINE_double(graph_scale, 1.0, "Graph/LM scale for language score");
DEFINE_bool(allow_partial, true, "If allow partial paths exists");
DEFINE_double(beam, 3.0, "Decoding beam");
DEFINE_int32(max_active, 200, "Max active states for one state");
DEFINE_int32(min_active, 0, "Min active states for one state");
DEFINE_int32(max_seq_len, 50, "Max sequence length of Transformer");
DEFINE_int32(sos, 5005, "Start of Sentence");
DEFINE_int32(eos, 5005, "End of Sentence");

int main(int argc,char* argv[]){

    const char *usage =
        "transformer faster decoder which decode features using Transformer-based model.\n"
        "\n"
        "Usage: transformer-decode-faster [options] <transformer-config> <graph-in> <feature-scp> <words-wspecifier>\n";

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
    float graph_scale = FLAGS_graph_scale;
    int32 max_seq_len = FLAGS_max_seq_len;
    int32 sos = FLAGS_sos;
    int32 eos = FLAGS_eos;


    std::string am_config = argv[1],
        fst_rxfilename = argv[2],
        feature_rspecifier = argv[3],
        words_wxfilename = argv[4];


    std::cout<<"acoustic_scale: "<<FLAGS_acoustic_scale<<std::endl;
    std::cout<<"graph_scale: "<<FLAGS_graph_scale<<std::endl;
    std::cout<<"allow_partial: "<<FLAGS_allow_partial<<std::endl;
    std::cout<<"beam: "<<FLAGS_beam<<std::endl;
    std::cout<<"max_active: "<<FLAGS_max_active<<std::endl;
    std::cout<<"min_active: "<<FLAGS_min_active<<std::endl;
    std::cout<<"max_seq_len: "<<FLAGS_max_seq_len<<std::endl;
    std::cout<<"sos: "<<FLAGS_sos<<std::endl;
    std::cout<<"eos: "<<FLAGS_eos<<std::endl;


    // read am
    
    
    void* model_handle; 
    if(inference::STATUS_OK != inference::LoadModel(am_config.c_str(),model_handle)){
        std::cerr<<"load am model failed";
        return -1;
    }

    void* inf_handle;
    if(inference::STATUS_OK != inference::CreateHandle(model_handle, inf_handle)){
        std::cerr<<"create am inference handle failed";
        return -1;
    }




    // read graph
    std::cout<<"start reading graph"<<std::endl;
    athena::StdVectorFst* pgraph=ReadGraph(fst_rxfilename);
    if(pgraph==NULL){
        std::cerr<<"read graph failed"<<std::endl;
        return -1;
    }
    std::cout<<"finish reading graph"<<std::endl;


    athena::TransformerFasterDecoderOptions options;
    options.beam=beam;
    options.max_active=max_active;
    options.min_active=min_active;
    options.max_seq_len=max_seq_len;
    options.graph_scale=graph_scale;

    athena::TransformerFasterDecoder decoder(*pgraph,options);

    std::ifstream feature_scp(feature_rspecifier,std::ios::in);
    if(!feature_scp.is_open()){
        std::cerr<<"open feature scp error"<<std::endl;
        return -1;
    }

    std::ofstream trans_out(words_wxfilename,std::ios::out);
    if(!trans_out.is_open()){
        std::cerr<<"open trans out error"<<std::endl;
        return -1;
    }

    std::vector<int> trans;
    std::string key, path;
    while(!feature_scp.eof()){
        std::cout<<"dealing key: "<<key<<std::endl;
        if(feature_scp>>key>>path){
            short* pcm_samples = NULL;
            int short_size = 0;
            ReadPCMFile(path.c_str(), &pcm_samples, &short_size);
            inference::Input in;
            in.pcm_raw = (char*) pcm_samples;
            in.pcm_size = short_size*2;

            TransformerDecodable decodable(inf_handle,acoustic_scale,sos,eos);
            decodable.GetEncoderOutput(&in);
            decoder.Decode(&decodable);
            trans.clear();
            decoder.GetBestPath(trans);
            WriteTrans(trans_out, key,trans);
        }

    }

    if(inference::STATUS_OK != inference::FreeHandle(inf_handle)){
        std::cerr<<"Destroy am handler failed!";
        return -1;
    }
    if(inference::STATUS_OK != inference::FreeModel(model_handle)){
        std::cerr<<"Unload am model Failed!!";
        return -1;
    }

    if(pgraph) delete pgraph;

    return 0;
}




