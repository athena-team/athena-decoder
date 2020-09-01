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
};

struct encoder_output{
    std::vector<float> enc_out;
    std::vector<int> enc_out_shape;
    std::vector<float> enc_mask;
    std::vector<int> enc_mask_shape;

    encoder_output(){
        enc_out.clear();
        enc_mask.clear();
        enc_out_shape.clear();
        enc_mask_shape.clear();
    }

    ~encoder_output(){
        enc_out.clear();
        enc_mask.clear();
        enc_out_shape.clear();
        enc_mask_shape.clear();
    }
};

struct PackedStates{
};

enum INFStatus {
    STATUS_OK      = 0,
    STATUS_ERROR   = -1,
};

PackedStates* GetInitialPackedStates(void* Inf_Handle);

INFStatus LoadModel(const char *conf, void* &Model_Handle);

INFStatus CreateHandle(void* Model_Handle, void* &Inf_Handle);

INFStatus GetEncoderOutput(void* Inf_Handle, Input* in, encoder_output* enc_output);

INFStatus InferenceOneStep(void* Inf_Handle, encoder_output* enc_output, std::vector<std::vector<int> >& batch_history_labels, 
                        PackedStates* current_packed_states, std::vector<std::vector<float> >& batch_log_scores,
                            PackedStates* packed_states);
INFStatus FreeHandle(void* Inf_Handle);
INFStatus FreeModel(void* Moldel_Handle);

} // namespace inference

#endif
