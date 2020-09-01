#ifndef ATTENTION_BEAMSEARCH_H
#define ATTENTION_BEAMSEARCH_H

#include "base.h"
#include "inference.h"
#include "tfmodel.h"
#include "FeatureExtraction.h"
#include <memory>

using namespace inference;

class Ctc {
public:
    Ctc();
    ~Ctc();
    int push_states(Input* in);
    int init(const char* config);
    int model_run(const TFModel* tfmodel);
    int get_new_states(encoder_output* output);
    TFModel* tfmodel;
    
private:
    // in
    DATA_TYPE<float> _input_seq;
    DATA_TYPE<float> _input_rnn_0;
    DATA_TYPE<float> _input_rnn_1;
    DATA_TYPE<float> _input_rnn_2;
    DATA_TYPE<float> _input_rnn_3;
    DATA_TYPE<float> _input_rnn_4;

    // out 
    DATA_TYPE<float> _output_state_0;
    DATA_TYPE<float> _output_state_1;
    DATA_TYPE<float> _output_state_2;
    DATA_TYPE<float> _output_state_3;
    DATA_TYPE<float> _output_state_4;
    DATA_TYPE<float> _logits;

    float* buffer_state_0;
    float* buffer_state_1;
    float* buffer_state_2;
    float* buffer_state_3;
    float* buffer_state_4;
   
    int pack_duration_; // seconds
    int left_context_;
    int right_context_;
    int repeat_size_;
    char *repeat_audio_;
    int remain_size_;
    char *remain_audio_;
    int frame_shift_size_;
    int sample_rate_;
    
    float *one_cal_feat_buff_;
    float *prefix_buff_;
    TFIO* tfio;

    FeatHandler *handler_;

    int feat_dim_;
    
};

#endif //end ATTENTION_BEAMSEARCH_H
