#include "decoder/transformer-argmax-decoder.h"
#include <algorithm>
#include <vector>
namespace athena {

TransformerArgmaxDecoder::TransformerArgmaxDecoder(int max_seq_len, int sos,int eos){
    this->max_seq_len = max_seq_len;
    this->sos = sos;
    this->eos = eos;
    this->trans.clear();
}
void TransformerArgmaxDecoder::Decode(TransformerDecodable* decodable){

    std::vector<std::vector<float> > batch_log_scores;
    std::vector<std::vector<int> > batch_history_labels;
    std::shared_ptr<inference::PackedStates> ps(NULL); 
    trans.push_back(sos);
    batch_history_labels.push_back(trans);

    for(int step=0;step<max_seq_len;step++){
        decodable->InferenceOneStep(batch_history_labels, ps, batch_log_scores, ps);
        std::vector<float> log_scores = batch_log_scores[0];
        std::vector<float>::iterator biggest = std::max_element(log_scores.begin(), log_scores.end());
        int biggest_idx = std::distance(std::begin(log_scores), biggest);
        trans.push_back(biggest_idx);
        if(biggest_idx==eos){
            break;
        }
    }

} 

void TransformerArgmaxDecoder::GetBestPath(std::vector<int>& trans){
    trans.clear();
    trans.assign(this->trans.begin()+1, this->trans.end()-1);
}

} // end namespace athena
