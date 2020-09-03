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

#include "ctc.h"

Ctc::Ctc(){
    std::vector<std::size_t> shape;
    // in
    _input_seq = std::make_tuple("input_feature", make_unique<Buffer<float> >(), shape);
    _input_rnn_0 = std::make_tuple("initial_0", make_unique<Buffer<float> >(), shape);
    _input_rnn_1 = std::make_tuple("initial_1", make_unique<Buffer<float> >(), shape);
    _input_rnn_2 = std::make_tuple("initial_2", make_unique<Buffer<float> >(), shape);
    _input_rnn_3 = std::make_tuple("initial_3", make_unique<Buffer<float> >(), shape);
    _input_rnn_4 = std::make_tuple("initial_4", make_unique<Buffer<float> >(), shape);
    // out
    _output_state_0 = std::make_tuple("state_0", make_unique<Buffer<float> >(), shape);
    _output_state_1 = std::make_tuple("state_1", make_unique<Buffer<float> >(), shape);
    _output_state_2 = std::make_tuple("state_2", make_unique<Buffer<float> >(), shape);
    _output_state_3 = std::make_tuple("state_3", make_unique<Buffer<float> >(), shape);
    _output_state_4 = std::make_tuple("state_4", make_unique<Buffer<float> >(), shape);
    _logits = std::make_tuple("logits", make_unique<Buffer<float>>(), shape);
    
    tfio = new TFIO;

    feat_dim_ = 40;    
    pack_duration_ = 2;
    repeat_size_ = 480 + 8 * 320;
    repeat_audio_ = NULL;
    remain_size_ = 0;
    remain_audio_ = new char[320];
    frame_shift_size_ = 320;
    sample_rate_ = 8000;

    buffer_state_0 = new float[512];
    buffer_state_1 = new float[512];
    buffer_state_2 = new float[512];
    buffer_state_3 = new float[512];
    buffer_state_4 = new float[512]; 
    
    one_cal_feat_buff_ = NULL;
    prefix_buff_ = NULL;
}

Ctc::~Ctc(){
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

int Ctc::push_states(Input* in) {
    tfio->clear();
    char *audio_tmp;
    int start_udt_index = 0;
    int total_audio_size = 0;
     
    if (in->first){
        std::memset(prefix_buff_, 0.0, 8*440*sizeof(float));
        std::memset(buffer_state_0, 0.0, 512*sizeof(float));
        std::memset(buffer_state_1, 0.0, 512*sizeof(float));
        std::memset(buffer_state_2, 0.0, 512*sizeof(float));
        std::memset(buffer_state_3, 0.0, 512*sizeof(float));
        std::memset(buffer_state_4, 0.0, 512*sizeof(float));
        if(NULL != repeat_audio_){
            std::memset(repeat_audio_, 0, repeat_size_);
        }
        remain_size_ = 0;
        total_audio_size = in->pcm_size; 
        audio_tmp = new char[total_audio_size];
        if(NULL == audio_tmp){
           std::cerr<<"new operator fail for audio_tmp"<<std::endl;
           return -1;
        }
        std::memcpy(audio_tmp, in->pcm_raw, total_audio_size);
        //update parameter
        remain_size_ = (in->pcm_size - repeat_size_ - frame_shift_size_) % frame_shift_size_;
        if (remain_size_ > 0){
            start_udt_index = in->pcm_size - remain_size_ - repeat_size_;
            std::memcpy(repeat_audio_, in->pcm_raw + start_udt_index, repeat_size_);
            // store remain audio
            std::memcpy(remain_audio_, in->pcm_raw + in->pcm_size - remain_size_, remain_size_);
        }else if (remain_size_ == 0){
            start_udt_index = in->pcm_size - repeat_size_;
            std::memcpy(repeat_audio_, in->pcm_raw + start_udt_index, repeat_size_);
        }else{
            remain_size_ = 0;
        }
    }else{
        if(remain_size_ > 0){
            total_audio_size = repeat_size_ + remain_size_ + in->pcm_size;
            audio_tmp = new char[total_audio_size];
            if(NULL == audio_tmp){
                std::cerr<<"new operator fail for audio_tmp"<<std::endl;
                return -1;
            }
            std::memcpy(audio_tmp, repeat_audio_, repeat_size_);
            std::memcpy(audio_tmp + repeat_size_, remain_audio_, remain_size_);
            std::memcpy(audio_tmp + repeat_size_ + remain_size_, in->pcm_raw, in->pcm_size);
        }else{
            total_audio_size = repeat_size_ + in->pcm_size;
            audio_tmp = new char[total_audio_size];
            if(NULL == audio_tmp){
                std::cerr<<"new operator fail for audio_tmp"<<std::endl;
                return -1;
            }
            std::memcpy(audio_tmp, repeat_audio_, repeat_size_);
            std::memcpy(audio_tmp + repeat_size_, in->pcm_raw, in->pcm_size);
        }
        // update parameter
        remain_size_ = (in->pcm_size + remain_size_) % frame_shift_size_;
        if (remain_size_ > 0){
            start_udt_index = in->pcm_size - remain_size_ - repeat_size_;
            if(start_udt_index >= 0){
                std::memcpy(repeat_audio_, in->pcm_raw + start_udt_index, repeat_size_);
                std::memcpy(remain_audio_, in->pcm_raw + in->pcm_size - remain_size_, remain_size_);
            }else{
                if(in->pcm_size >= remain_size_){
                    int short_repeat_size = in->pcm_size - remain_size_;
                    std::memmove(repeat_audio_, repeat_audio_ + short_repeat_size, repeat_size_ - short_repeat_size);
                    std::memcpy(repeat_audio_ + repeat_size_ - short_repeat_size, in->pcm_raw, short_repeat_size);
                    std::memcpy(remain_audio_, in->pcm_raw + in->pcm_size - remain_size_, remain_size_);
                }else{
                    // repeat audio not change, only add more remain_audio
                    int old_remain = remain_size_ - in->pcm_size;
                    std::memcpy(remain_audio_ + old_remain, in->pcm_raw, in->pcm_size);
                }
            }
        }else{
            start_udt_index = in->pcm_size - repeat_size_;
            if (start_udt_index >= 0){
                std::memcpy(repeat_audio_, in->pcm_raw + start_udt_index, repeat_size_);
            }else{
                std::memmove(repeat_audio_, repeat_audio_ + in->pcm_size, repeat_size_ - in->pcm_size);
                std::memcpy(repeat_audio_ + repeat_size_ - in->pcm_size, in->pcm_raw, in->pcm_size);
            }
        }
    }
    if (total_audio_size < 160){
        std::cerr << "pcm size is too short " << total_audio_size << "\n";
        return -1;
    }  
    int feat_size = 0;
    int frame_feat_size = handler_->get_feat_size();
    int feat_dim = frame_feat_size/sizeof(float);
    int max_feat_size = handler_->get_max_output_size();
    char *feat_buff = new char[max_feat_size];
    handler_->feat_extract(audio_tmp, total_audio_size, feat_buff, feat_size);  
    int feat_num = feat_size / sizeof(float);
    int frame_num = feat_num / feat_dim_;
    float *input_features = (float*) feat_buff;
    int time_steps = 0;
    if (in->first){
        if (frame_num < 4){
            std::cerr << "the first packet is too short!\n";
            return -1; 
        }
        if (in->last){
            time_steps = frame_num;
        }else{
            time_steps = frame_num - right_context_;
        }
    } else{
        if (!in->last && frame_num < 1){
            std::cerr << "the last packet is too short!\n";
            return -1;
        }
        if(in->last){
            time_steps = frame_num + right_context_;
        } else{
            time_steps = frame_num;
        }
    }
    handler_->cmvn_feat(input_features, feat_num);
    // input features
    std::vector<std::size_t> &shape_input_seq =  get_shape(_input_seq);
    shape_input_seq.clear();
    shape_input_seq.push_back(1);
    shape_input_seq.push_back(time_steps);
    shape_input_seq.push_back(440);
    shape_input_seq.push_back(1);
  
    const std::unique_ptr<Buffer<float> > &buffer_input_seq = get_buffer(_input_seq);
    if (in->first){
        buffer_input_seq->resize(time_steps*440); 
        for (int i=0; i<left_context_; i++) {
            std::memcpy(one_cal_feat_buff_ + i * feat_dim_, input_features, feat_dim_ * sizeof(float));
        } 
        std::memcpy(one_cal_feat_buff_ + left_context_ * feat_dim_, input_features, (right_context_ + 1) * feat_dim_ * sizeof(float));
        std::memcpy(buffer_input_seq->ptr() , one_cal_feat_buff_, (left_context_ + 1 + right_context_) * feat_dim_ * sizeof(float));
        input_features += (right_context_ + 1) * feat_dim_;
        int i=1;
        for (; i<frame_num - right_context_; i++) {
            std::memmove(one_cal_feat_buff_, one_cal_feat_buff_ + feat_dim, (left_context_ + right_context_) * feat_dim_ * sizeof(float));
            std::memcpy(one_cal_feat_buff_ + (left_context_ + right_context_) * feat_dim, input_features, feat_dim_ * sizeof(float));
            std::memcpy(buffer_input_seq->ptr() + i * (left_context_ + right_context_ + 1) * feat_dim_, one_cal_feat_buff_, (left_context_ + 1 + right_context_) * feat_dim_ * sizeof(float)); 
            input_features += feat_dim_;
        }
        if (in->last){
            for (; i<frame_num - 1; i++){
                std::memmove(one_cal_feat_buff_, one_cal_feat_buff_ + feat_dim, (left_context_ + right_context_) * feat_dim_ * sizeof(float));
                std::memcpy(buffer_input_seq->ptr() + i * (left_context_ + right_context_ + 1) * feat_dim_, one_cal_feat_buff_, (left_context_ + 1 + right_context_) * feat_dim_ * sizeof(float));
            }
        } else{
            std::memcpy(prefix_buff_, buffer_input_seq->ptr() + (time_steps - 8)*440, 8 * 440 * sizeof(float));
        }
    } else{
        shape_input_seq[1] += 8;
        buffer_input_seq->resize((time_steps + 8) * 440);
        std::memcpy(buffer_input_seq->ptr(), prefix_buff_, 8 * 440 * sizeof(float));
        for(int i=0; i<frame_num; i++) {
            std::memmove(one_cal_feat_buff_, one_cal_feat_buff_ + feat_dim, (left_context_ + right_context_) * feat_dim_ * sizeof(float));
            std::memcpy(one_cal_feat_buff_ + (left_context_ + right_context_) * feat_dim, input_features, feat_dim_ * sizeof(float));
            std::memcpy(buffer_input_seq->ptr() + 8 * 440 + i * (left_context_ + right_context_ + 1) * feat_dim_, one_cal_feat_buff_, (left_context_ + 1 + right_context_) * feat_dim_ * sizeof(float));
            input_features += feat_dim_;
        }
        if (in->last){
            for (int i=0; i<right_context_; i++){
                std::memmove(one_cal_feat_buff_, one_cal_feat_buff_ + feat_dim, (left_context_ + right_context_) * feat_dim_ * sizeof(float));
                std::memcpy(buffer_input_seq->ptr() + 8 * 440 + (i + frame_num) * (left_context_ + right_context_ + 1) * feat_dim_, one_cal_feat_buff_, (left_context_ + 1 + right_context_) * feat_dim_ * sizeof(float));
            }
        } else{
            std::memcpy(prefix_buff_, buffer_input_seq->ptr() + time_steps*440, 8 * 440 * sizeof(float));
        }
    }
    
    // input rnn states
    std::vector<std::size_t> &shape_input_rnn_0 = get_shape(_input_rnn_0);
    shape_input_rnn_0.clear();
    shape_input_rnn_0.push_back(1);
    shape_input_rnn_0.push_back(512);
    const std::unique_ptr<Buffer<float> > &buffer_input_rnn_0 = get_buffer(_input_rnn_0);
    buffer_input_rnn_0->resize(512);
    std::memcpy(buffer_input_rnn_0->ptr(), buffer_state_0, 512*sizeof(float));
    std::vector<std::size_t> &shape_input_rnn_1 = get_shape(_input_rnn_1);
    shape_input_rnn_1.clear();
    shape_input_rnn_1.push_back(1);
    shape_input_rnn_1.push_back(512);
    const std::unique_ptr<Buffer<float> > &buffer_input_rnn_1 = get_buffer(_input_rnn_1);
    buffer_input_rnn_1->resize(512);
    std::memcpy(buffer_input_rnn_1->ptr(), buffer_state_1, 512*sizeof(float));    
    std::vector<std::size_t> &shape_input_rnn_2 = get_shape(_input_rnn_2);
    shape_input_rnn_2.clear();
    shape_input_rnn_2.push_back(1);
    shape_input_rnn_2.push_back(512);
    const std::unique_ptr<Buffer<float> > &buffer_input_rnn_2 = get_buffer(_input_rnn_2);
    buffer_input_rnn_2->resize(512);
    std::memcpy(buffer_input_rnn_2->ptr(), buffer_state_2, 512*sizeof(float)); 
    std::vector<std::size_t> &shape_input_rnn_3 = get_shape(_input_rnn_3);
    shape_input_rnn_3.clear();
    shape_input_rnn_3.push_back(1);
    shape_input_rnn_3.push_back(512);
    const std::unique_ptr<Buffer<float> > &buffer_input_rnn_3 = get_buffer(_input_rnn_3);
    buffer_input_rnn_3->resize(512);
    std::memcpy(buffer_input_rnn_3->ptr(), buffer_state_3, 512*sizeof(float));
    std::vector<std::size_t> &shape_input_rnn_4 = get_shape(_input_rnn_4);
    shape_input_rnn_4.clear();
    shape_input_rnn_4.push_back(1);
    shape_input_rnn_4.push_back(512);
    const std::unique_ptr<Buffer<float> > &buffer_input_rnn_4 = get_buffer(_input_rnn_4);
    buffer_input_rnn_4->resize(512);
    std::memcpy(buffer_input_rnn_4->ptr(), buffer_state_4, 512*sizeof(float));
    delete[] feat_buff;
    delete[] audio_tmp;
    tfio->set_input(_input_seq);
    tfio->set_input(_input_rnn_0);
    tfio->set_input(_input_rnn_1);
    tfio->set_input(_input_rnn_2);
    tfio->set_input(_input_rnn_3);
    tfio->set_input(_input_rnn_4);
    tfio->set_output(get_name(_output_state_0));
    tfio->set_output(get_name(_output_state_1));
    tfio->set_output(get_name(_output_state_2));
    tfio->set_output(get_name(_output_state_3));
    tfio->set_output(get_name(_output_state_4));
    tfio->set_output(get_name(_logits));
}

int Ctc::init(const char* config) {

    string entry_conf(config);
    int pos = entry_conf.find_last_of('/');
    string conf_dir = entry_conf.substr(0, pos+1);
    char feat_config[1024];
    char cmvn_config[1024];
    FILE *fIconf = fopen(config, "r" );
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
            }else if(0 == strcmp("SAMPLERATE", conf_key)){
                sample_rate_ = atoi(conf_value);
            }else if(0 == strcmp("CONTEXT_LEFT", conf_key)){
                left_context_ = atoi(conf_value); 
            }else if(0 == strcmp("CONTEXT_RIGHT", conf_key)){
                right_context_ = atoi(conf_value);
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

    if (sample_rate_ == 8000){
        repeat_size_ = 240;
        frame_shift_size_ = 160;
 
    } 
    repeat_audio_ = new char[repeat_size_];
    if(NULL == repeat_audio_){
        std::cerr<<"new operator fail for repeat_audio_"<<std::endl;
        return -1;
    }
    std::memset(repeat_audio_, 0, repeat_size_);
    one_cal_feat_buff_ = new float[(left_context_ + right_context_ + 1)*feat_dim_];
    prefix_buff_ = new float[8*(left_context_ + right_context_ + 1)*feat_dim_]; 
    return 0;
}

int Ctc::model_run(const TFModel* tfmodel) {
    // Session run
    RunOptions run_options;
    Status s = tfmodel->session->Run(run_options, tfio->input_tensors, tfio->output_names, {}, &(tfio->output_tensors), nullptr); 
    if (!s.ok()){
        std::cerr << "Error during TFModel run: " << s;
        return -1;
    }
    return 0;
}

int Ctc::get_new_states(encoder_output* output) {
    int index_ = tfio->get_index(tfio->output_names, get_name(_output_state_0));
    if (index_ == -1){
        return -1;
    }
    Tensor &state_tensor_0 = tfio->output_tensors[index_];
    auto data_state_0 = state_tensor_0.flat<float>();
    std::size_t num_out_tensor = state_tensor_0.NumElements();
    for (int i=0; i<num_out_tensor; i++){
        buffer_state_0[i] = data_state_0(i);
    }
    index_ = tfio->get_index(tfio->output_names, get_name(_output_state_1));
    if (index_ == -1){
        return -1;
    }
    Tensor &state_tensor_1 = tfio->output_tensors[index_];
    auto data_state_1 = state_tensor_1.flat<float>();
    num_out_tensor = state_tensor_1.NumElements();
    for (int i=0; i<num_out_tensor; i++){
        buffer_state_1[i] = data_state_1(i);
    }
    index_ = tfio->get_index(tfio->output_names, get_name(_output_state_2));
    if (index_ == -1){
        return -1;
    }
    Tensor &state_tensor_2 = tfio->output_tensors[index_];
    auto data_state_2 = state_tensor_2.flat<float>();
    num_out_tensor = state_tensor_2.NumElements();
    for (int i=0; i<num_out_tensor; i++){
        buffer_state_2[i] = data_state_2(i);
    }
    index_ = tfio->get_index(tfio->output_names, get_name(_output_state_3));
    if (index_ == -1){
        return -1;
    }
    Tensor &state_tensor_3 = tfio->output_tensors[index_];
    auto data_state_3 = state_tensor_3.flat<float>();
    num_out_tensor = state_tensor_3.NumElements();
    for (int i=0; i<num_out_tensor; i++){
        buffer_state_3[i] = data_state_3(i);
    }
    index_ = tfio->get_index(tfio->output_names, get_name(_output_state_4));
    if (index_ == -1){
        return -1;
    }
    Tensor &state_tensor_4 = tfio->output_tensors[index_];
    auto data_state_4 = state_tensor_4.flat<float>();
    num_out_tensor = state_tensor_4.NumElements();
    for (int i=0; i<num_out_tensor; i++){
        buffer_state_4[i] = data_state_4(i);
    }
    index_ = tfio->get_index(tfio->output_names, get_name(_logits));
    if (index_ == -1){
        return -1;
    }
    Tensor &logits_tensor = tfio->output_tensors[index_];
    auto data_logits = logits_tensor.flat<float>();
    num_out_tensor = logits_tensor.NumElements();
    const TensorShape &ts_logits_shape = logits_tensor.shape();
    std::vector<int> logits_shape;
    for (auto it = ts_logits_shape.begin(); it != ts_logits_shape.end(); ++it) {
        logits_shape.push_back((*it).size);
    }
    output->logits.clear();
    for (int i=0; i<logits_shape[1]; i++){
        std::vector<float> tmp;
        for (int j=0; j<logits_shape[2]; j++){
            tmp.push_back(data_logits(i*logits_shape[2] + j));
        }
        output->logits.push_back(tmp);
    }
    return 0;
}



