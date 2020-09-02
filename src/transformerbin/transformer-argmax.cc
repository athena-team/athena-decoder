#include <cstring>
#include "decoder/transformer-argmax-decoder.h"
#include "transformer/decodable-am-transformer.h"
#include "utils/utils.h"
#include "transformer/inference.h"
#include "gflags/gflags.h"

DEFINE_int32(max_seq_len, 50, "Max sequence length of Transformer");
DEFINE_int32(sos, 5005, "Start of Sentence");
DEFINE_int32(eos, 5005, "End of Sentence");

int main(int argc,char* argv[]){

    const char *usage =
        "transformer argmax decoder which decode features using Transformer-based model.\n"
        "\n"
        "Usage: transformer-decode-faster [options] <transformer-config> <feature-scp> <words-wspecifier>\n";

    google::ParseCommandLineFlags(&argc, &argv, true);
    if (argc != 4) {
        std::cout << usage << std::endl;
        return 0;
    }
    int32 max_seq_len = FLAGS_max_seq_len;
    int32 sos = FLAGS_sos;
    int32 eos = FLAGS_eos;
    std::string am_config = argv[1],
        feature_rspecifier = argv[2],
        words_wxfilename = argv[3];
    
    std::cout<<"max_seq_len: "<<max_seq_len<<std::endl;
    std::cout<<"sos: "<<sos<<std::endl;
    std::cout<<"eos: "<<eos<<std::endl;

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


    athena::TransformerArgmaxDecoder decoder(max_seq_len, sos, eos);

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

            TransformerDecodable decodable(inf_handle,1.0,sos,eos);
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
    return 0;
}




