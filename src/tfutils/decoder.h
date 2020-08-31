#ifndef DECODER_H
#define DECODER_H

#include "base.h"
#include "inference.h"
#include "tfmodel.h"
#include <memory>

using namespace inference;


class Decoder {
public:
    Decoder();
    ~Decoder();
    int push_states(encoder_output* enc_output, std::vector<std::vector<int> >& history_labels);
    int init_decoder();
    int model_run(const TFModel* tfmodel);
    int get_new_states(std::vector<std::vector<float> >& batch_log_scores);
    TFModel* tfmodel;

private:
    // in
    DATA_TYPE<float> _decoder_encoder_output;
    DATA_TYPE<float> _decoder_encoder_output_mask;
    DATA_TYPE<int> _decoder_step;
    DATA_TYPE<float> _decoder_history_pred;

    TFIO* tfio;
    // out 
    DATA_TYPE<float> _out_log_scores;

};





#endif //end DECODER_H
