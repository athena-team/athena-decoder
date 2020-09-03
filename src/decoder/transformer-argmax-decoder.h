#ifndef TRANSFORMER_ARGMAX_DECODER_H_
#define TRANSFORMER_ARGMAX_DECODER_H_

#include <memory>
#include <algorithm>
#include "transformer/decodable-am-transformer.h"
namespace athena {

class TransformerArgmaxDecoder{
public:
    TransformerArgmaxDecoder(int max_seq_len,int sos,int eos);
    void Decode(TransformerDecodable* decodable);
    void GetBestPath(std::vector<int>& trans);
private:
    int max_seq_len;
    void* inf_handle;
    int sos;
    int eos;
    std::vector<int> labels;
};

} 


#endif
