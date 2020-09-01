#ifndef ENCODER_H
#define ENCODER_H

#include "base.h"
#include "inference.h"
#include "tfmodel.h"
#include "FeatureExtraction.h"
#include <memory>

using namespace inference;

class Encoder {
public:
    Encoder();
    ~Encoder();
    int push_states(Input* in);
    int init_encoder(const char* enc_config);
    int model_run(const TFModel* tfmodel);
    int get_new_states(encoder_output* enc_output);
    TFModel* tfmodel;
    
private:
    // in
    DATA_TYPE<int> _encoder_input_len;
    DATA_TYPE<float> _encoder_input;

    // out 
    DATA_TYPE<int> _out_mask;
    DATA_TYPE<float> _out_enc;

    TFIO* tfio;

    FeatHandler *handler_;

    int feat_dim_;
    
};

#endif //end ENCODER_H
