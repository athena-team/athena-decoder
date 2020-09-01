#ifndef INFERENCE_H
#define INFERENCE_H
#include <cstring>
#include <stdlib.h>
#include <string>
#include <vector>


namespace inference
{

struct Input{
    char* pcm_raw;
    int pcm_size;
    bool first;
    bool last;
};

struct encoder_output{
    std::vector<float> enc_out;
    std::vector<int> enc_out_shape;
    std::vector<float> enc_mask;
    std::vector<int> enc_mask_shape;
    std::vector<std::vector<float> > logits;

    encoder_output(){
        enc_out.clear();
        enc_mask.clear();
        enc_out_shape.clear();
        enc_mask_shape.clear();
        logits.clear();
    }
    void clear(){
        enc_out.clear();
        enc_mask.clear();
        enc_out_shape.clear();
        enc_mask_shape.clear();
        logits.clear();
    }

    ~encoder_output(){
        enc_out.clear();
        enc_mask.clear();
        enc_out_shape.clear();
        enc_mask_shape.clear();
        logits.clear();
    }
};

enum INFStatus {
    STATUS_OK      = 0,
    STATUS_ERROR   = -1,
};

INFStatus LoadModel(const char *conf, void* &Model_Handle);

INFStatus CreateHandle(void* Model_Handle, void* &Inf_Handle);

INFStatus GetEncoderOutput(void* Inf_Handle, Input* in, encoder_output* enc_output);

INFStatus FreeHandle(void* Inf_Handle);
INFStatus FreeModel(void* Moldel_Handle);

} // namespace inference

#endif
