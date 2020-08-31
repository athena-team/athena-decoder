#include "type.h"
#include "buffer.h"
#include "encoder.h"
#include "decoder.h"
#include "inference.h"
#include <string>
#include <cstring>
#include "operator.h"

namespace inference
{

struct ModelHandle{
    TFModel* enc_model;
    TFModel* dec_model;
    ~ModelHandle(){
        if (NULL!=enc_model) { delete enc_model; enc_model=NULL; }
        if (NULL!=dec_model) { delete dec_model; dec_model=NULL; }
    }
};

INFStatus LoadModel(const char *conf, void* &Model_Handle) {
    string entry_conf(conf);
    int pos = entry_conf.find_last_of('/');
    string conf_dir = entry_conf.substr(0, pos+1);
    char enc_config[1024];
    std::string enc_path;
    std::string dec_path;
    int num_threads;
    FILE *fIconf = fopen(conf, "r" );
    if(NULL == fIconf) {
        std::cerr<<"Fail to load handle conf"<<std::endl;
        return STATUS_ERROR;
    }else {
        char conf_key[1024], conf_value[1024];
        while(-1 != fscanf( fIconf, "%s %s", conf_key, conf_value )){
            if(0 == strcmp("",conf_key)){
                continue;
            }
            std::string tmp_dir(conf_dir);
            if(0 == strcmp("ENCCONF",conf_key)){
                strcpy(enc_config, conf_value);
            }else if(0 == strcmp("ENCODER", conf_key)){
                enc_path.assign(conf_value);
            }else if(0 == strcmp("DECODER", conf_key)){
                dec_path.assign(conf_value);
            }else if(0 == strcmp("NUM_THREADS", conf_key)){
                num_threads = std::atoi(conf_value);
            }
        }
    }
    fclose(fIconf);

    ModelHandle *model_handle = new ModelHandle;

    // init encoder
    model_handle->enc_model = init_model(enc_path, num_threads);
    strcpy(model_handle->enc_model->config, enc_config);

    // init decoder
    model_handle->dec_model = init_model(dec_path, num_threads);

    Model_Handle = (void*)model_handle;

    return STATUS_OK;
}

INFStatus CreateHandle(void* Model_Handle, void* &Inf_Handle) {
    ModelHandle *model_handle = (ModelHandle *)Model_Handle;
    InfHandle *inf_handle = new InfHandle;

    // encoder
    Encoder* enc = new Encoder();
    if (NULL == enc){
        std::cerr << "Fail to new encoder object!" << std::endl;
        return STATUS_ERROR;
    }

    inf_handle->enc_handle = enc;
    inf_handle->enc_handle->tfmodel = model_handle->enc_model;

    int ret;
    ret = enc->init_encoder(model_handle->enc_model->config);

    if (-1 == ret){
        std::cerr << "Fail to init enc handle!" << std::endl;
        return STATUS_ERROR;
    }
    
    // decoder
    Decoder* dec = new Decoder();
    if (NULL == dec){
        std::cerr << "Fail to new decoder object!" << std::endl;
        return STATUS_ERROR;
    }
    inf_handle->dec_handle = dec;
    inf_handle->dec_handle->tfmodel = model_handle->dec_model;
    ret = dec->init_decoder();

    if (-1 == ret) {
        std::cerr << "Fail to init dec handle!" << std::endl;
        return STATUS_ERROR;
    }

    Inf_Handle = (void*)inf_handle;
    
    return STATUS_OK;
}

INFStatus GetEncoderOutput(void* Inf_Handle, Input* in, encoder_output* enc_output){
    InfHandle *inf_handle = (InfHandle *)Inf_Handle;
    Encoder* enc_ = (Encoder*)inf_handle->enc_handle;
    enc_->push_states(in);
    int ret = enc_->model_run(enc_->tfmodel);
    if (-1 == ret ){
        std::cerr << "Fail to enc model run!" << std::endl;
        return STATUS_ERROR;
    }
    ret = 0;
    ret = enc_->get_new_states(enc_output);
    if (-1 == ret){
        std::cerr << "Fail to get enc new states!" << std::endl;
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

PackedStates* GetInitialPackedStates(void* Inf_Handel){
    return NULL;
}

INFStatus InferenceOneStep(void* Inf_Handle, encoder_output* enc_output, std::vector<std::vector<int> >& batch_history_labels, 
                        PackedStates* current_packed_states, std::vector<std::vector<float> >& batch_log_scores,
                            PackedStates* packed_states) {
    InfHandle *inf_handle = (InfHandle *)Inf_Handle;
    Decoder* dec_ = inf_handle->dec_handle;
    dec_->push_states(enc_output, batch_history_labels);
    int ret = dec_->model_run(dec_->tfmodel);
    if (-1 == ret){
        std::cerr << "Fail to dec model run!" << std::endl;
        return STATUS_ERROR;
    }
    ret = 0;
    ret = dec_->get_new_states(batch_log_scores);
    if (-1 == ret){
        std::cerr << "Fail to get dec new states!" << std::endl;
        return STATUS_ERROR;
    }
    return STATUS_OK;    
}

INFStatus FreeHandle(void* Inf_Handle) {
    InfHandle *handle_ = (InfHandle *)Inf_Handle;
    if(NULL!=handle_){
        delete handle_;
        handle_=NULL;
    } 
    return STATUS_OK;
}

INFStatus FreeModel(void* Model_Handle) {
   ModelHandle *model_handle = (ModelHandle *)Model_Handle; 
   if (NULL != model_handle){
       delete model_handle;
       model_handle = NULL;
   } 
   return STATUS_OK;
}

}
