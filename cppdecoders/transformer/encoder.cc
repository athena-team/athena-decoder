// Copyright (C) 2019 ATHENA DECODER AUTHORS; Xiangang Li; Yang Han; Long Yuan
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ==============================================================================

#include "encoder.h"

Encoder::Encoder(){
    std::vector<std::size_t> shape;
    // in
    _encoder_input = std::make_tuple("input_features_", make_unique<Buffer<float> >(), shape);
    _encoder_input_len = std::make_tuple("input_features_len", make_unique<Buffer<int> >(), shape);

    // out
    _out_mask = std::make_tuple("mask", make_unique<Buffer<int>>(), shape);
    _out_enc = std::make_tuple("enc_output", make_unique<Buffer<float>>(), shape); // 0407 
    tfio = new TFIO;

    feat_dim_ = 40;    
}

Encoder::~Encoder(){
    if (NULL != tfio){
        delete tfio;
        tfio = NULL;
    }

    if (NULL != tfmodel){
        tfmodel = NULL;
    }

    if (NULL != handler_){
        delete handler_;
        handler_ = NULL;
    }
}

int Encoder::push_states(Input* in) {
    tfio->clear();
    // get features and frames
    int max_feat_size = handler_->get_max_output_size();
    char *feat_buff = new char[max_feat_size];
    int feat_size;
    float* input_features;
    handler_->feat_extract(in->pcm_raw, in->pcm_size, feat_buff, feat_size);
    input_features = (float*)feat_buff;
    int feats_num = feat_size / sizeof(float);
    int frames_num = feats_num / feat_dim_;
    handler_->cmvn_feat(input_features, feats_num);    
    // encoder input frame len
    std::vector<std::size_t> &shape_encoder_input_len = get_shape(_encoder_input_len);
    shape_encoder_input_len.clear();
    shape_encoder_input_len.push_back(1); 
    const std::unique_ptr<Buffer<int> > &buffer_encoder_input_len = get_buffer(_encoder_input_len);
    buffer_encoder_input_len->resize(1);
    buffer_encoder_input_len->ptr()[0] = frames_num;

    // encoder input features
    // num of features
    std::vector<std::size_t> &shape_encoder_input =  get_shape(_encoder_input);
    shape_encoder_input.clear();
    shape_encoder_input.push_back(1);
    shape_encoder_input.push_back(frames_num);
    shape_encoder_input.push_back(feat_dim_);
    shape_encoder_input.push_back(1);
    
    const std::unique_ptr<Buffer<float> > &buffer_encoder_input = get_buffer(_encoder_input);
    buffer_encoder_input->resize(feat_dim_*frames_num);
    
    std::memcpy(buffer_encoder_input->ptr(), input_features, feat_dim_*frames_num*sizeof(float));

    delete[] feat_buff;
    tfio->set_input(_encoder_input);
    tfio->set_input(_encoder_input_len);
    tfio->set_output(get_name(_out_mask));
    tfio->set_output(get_name(_out_enc));
}

int Encoder::init_encoder(const char* enc_conf) {

    string entry_conf(enc_conf);
    int pos = entry_conf.find_last_of('/');
    string conf_dir = entry_conf.substr(0, pos+1);

    char feat_config[1024];
    char cmvn_config[1024];
    FILE *fIconf = fopen(enc_conf, "r" );
    if(NULL == fIconf) {
        std::cerr<<"Fail to load enc model conf"<<std::endl;
        return -1;
    }else {
        char conf_key[1024], conf_value[1024];
        while(-1 != fscanf( fIconf, "%s %s", conf_key, conf_value )){
            if(0 == strcmp("",conf_key)){
                continue;
            }

            std::string tmp_dir(conf_dir);
            if(0 == strcmp("FEAT_CONFIG",conf_key)){
                strcpy(feat_config, conf_value);
            }else if(0 == strcmp("CMVN_CONFIG",conf_key)){
                strcpy(cmvn_config, conf_value);
            }else if(0 == strcmp("FEAT_DIM", conf_key)){
                feat_dim_ = atoi(conf_value);
            }
        }
    }
    fclose(fIconf);

    handler_ = new FeatHandler();
    if (NULL == handler_) {
        std::cerr << "fail to new feat handler" << std::endl;
        return -1;
    }

    int ret = handler_->init(feat_config);
    if (0 != ret) {
        std::cerr<<"Fail to init FeatHandler"<<std::endl;
        return -1;
    }

    handler_->read_cmvn_params(cmvn_config); 

    return 0;
}

int Encoder::model_run(const TFModel* tfmodel) {
    // Session run
    RunOptions run_options;
    Status s = tfmodel->session->Run(run_options, tfio->input_tensors, tfio->output_names, {}, &(tfio->output_tensors), nullptr); 
    if (!s.ok()){
        std::cerr << "Error during TFModel run: " << s;
        return -1;
    }
    return 0;
}

int Encoder::get_new_states(encoder_output* enc_output) {
    int index = tfio->get_index(tfio->output_names, get_name(_out_enc));
    if (index == -1){
        return -1;
    }
    Tensor &out_tensor = tfio->output_tensors[index];
    auto data_out = out_tensor.flat<float>();
    std::size_t num_out_tensor = out_tensor.NumElements();
    enc_output->enc_out.clear();
    for (int i=0; i<num_out_tensor; i++){
        enc_output->enc_out.push_back(data_out(i));
    }
    
    const TensorShape &ts_out = out_tensor.shape();
    enc_output->enc_out_shape.clear();
    for (auto it = ts_out.begin(); it != ts_out.end(); ++it) {
        enc_output->enc_out_shape.push_back((*it).size);
    }
    
    index = tfio->get_index(tfio->output_names, get_name(_out_mask));
    if (index == -1){
        return -1;
    }

    Tensor &mask_tensor = tfio->output_tensors[index];
    auto data_mask = mask_tensor.flat<float>();
    std::size_t num_mask_tensor = mask_tensor.NumElements();
    enc_output->enc_mask.clear();
    for (int i=0; i<num_mask_tensor; i++){
        enc_output->enc_mask.push_back(data_mask(i));
    }

    const TensorShape &ts_mask = mask_tensor.shape();
    enc_output->enc_mask_shape.clear();
    for (auto it = ts_mask.begin(); it != ts_mask.end(); ++it) {
        enc_output->enc_mask_shape.push_back((*it).size);
    }
}
