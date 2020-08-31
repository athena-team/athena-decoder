#include "decoder.h"

Decoder::Decoder(){
    std::vector<std::size_t> shape;
    // in
    _decoder_encoder_output = std::make_tuple("in_encoder_output", make_unique<Buffer<float> >(), shape);
    _decoder_encoder_output_mask = std::make_tuple("in_encoder_output_mask", make_unique<Buffer<float> >(), shape);
    _decoder_step = std::make_tuple("in_step", make_unique<Buffer<int> >(), shape);
    _decoder_history_pred = std::make_tuple("in_history_pred", make_unique<Buffer<float> >(), shape);

    // out
    _out_log_scores = std::make_tuple("logits", make_unique<Buffer<float>>(), shape);

    tfio = new TFIO;
}

Decoder::~Decoder(){
    if (NULL != tfio){
        delete tfio;
        tfio = NULL;
    }

    if (NULL != tfmodel){
        tfmodel = NULL;
    }
}

int Decoder::push_states(encoder_output* enc_output, std::vector<std::vector<int> >& history_labels){
    tfio->clear();
    // decoder input decoder out
    int num_enc_out = 1;
    int batch = history_labels.size();
    std::vector<std::size_t> &shape_decoder_encoder_output = get_shape(_decoder_encoder_output);
    shape_decoder_encoder_output.clear();
    for (int i=0; i<enc_output->enc_out_shape.size(); i++){
        if (i == 0){
            shape_decoder_encoder_output.push_back(batch);
        }else{
            shape_decoder_encoder_output.push_back(enc_output->enc_out_shape[i]);
            num_enc_out *= enc_output->enc_out_shape[i];
        }
    }
    const std::unique_ptr<Buffer<float> > &buffer_decoder_encoder_output = get_buffer(_decoder_encoder_output);
    buffer_decoder_encoder_output->resize(batch*num_enc_out);
    for (int i=0; i<batch*num_enc_out; i++){
        buffer_decoder_encoder_output->ptr()[i] = enc_output->enc_out[i%num_enc_out];
    }
    
    // decoder input decoder out mask
    int num_enc_mask = 1;
    std::vector<std::size_t> &shape_decoder_encoder_output_mask = get_shape(_decoder_encoder_output_mask);
    shape_decoder_encoder_output_mask.clear();   
    for (int i=0; i<enc_output->enc_mask_shape.size(); i++){
        if (i==0){
            shape_decoder_encoder_output_mask.push_back(batch);
        }else{
            shape_decoder_encoder_output_mask.push_back(enc_output->enc_mask_shape[i]);
            num_enc_mask *= enc_output->enc_mask_shape[i];
        }
    }
    const std::unique_ptr<Buffer<float> > &buffer_decoder_encoder_output_mask = get_buffer(_decoder_encoder_output_mask);
    buffer_decoder_encoder_output_mask->resize(batch*num_enc_mask); 
    for (int i=0; i<batch*num_enc_mask; i++){
        buffer_decoder_encoder_output_mask->ptr()[i] = enc_output->enc_mask[i%num_enc_mask];
    }

    // decoder input decoder step
    int step = history_labels[0].size();
    std::vector<std::size_t> &shape_decoder_step = get_shape(_decoder_step);
    shape_decoder_step.clear();
    shape_decoder_step.push_back(batch);
    const std::unique_ptr<Buffer<int> > &buffer_decoder_step = get_buffer(_decoder_step);
    buffer_decoder_step->resize(batch);
    for(int i=0; i<batch; i++){
        buffer_decoder_step->ptr()[i] = step;
    }
    
    // decoder input decoder history labels
    std::vector<std::size_t> &shape_decoder_history_pred = get_shape(_decoder_history_pred);
    shape_decoder_history_pred.clear();
    shape_decoder_history_pred.push_back(batch);
    shape_decoder_history_pred.push_back(step);

    const std::unique_ptr<Buffer<float> > &buffer_decoder_history_pred = get_buffer(_decoder_history_pred);
    buffer_decoder_history_pred->resize(batch*step);
     
    for (int i=0; i<batch; i++) {
        for (int j=0; j<step; j++) {
            buffer_decoder_history_pred->ptr()[i*step + j] = history_labels[i][j];
        }
    }
    

    tfio->set_input(_decoder_encoder_output);
    tfio->set_input(_decoder_encoder_output_mask);
    tfio->set_input(_decoder_step);
    tfio->set_input(_decoder_history_pred);
    tfio->set_output(get_name(_out_log_scores));

}

int Decoder::init_decoder() {
    return 0;
}

int Decoder::model_run(const TFModel* tfmodel) {
    // Session run
    RunOptions run_options;

    Status s = tfmodel->session->Run(run_options, tfio->input_tensors, tfio->output_names, {}, &(tfio->output_tensors), nullptr);
    if (!s.ok()){
        std::cerr << "Error during TFModel run: " << s;
        return -1;
    }
    return 0;
}

int Decoder::get_new_states(std::vector<std::vector<float> >& batch_log_scores) {
    int index = tfio->get_index(tfio->output_names, get_name(_out_log_scores));
    if (index == -1){
        return -1;
    }

    Tensor &out_tensor = tfio->output_tensors[index];
    auto data_scores = out_tensor.flat<float>();
    std::size_t num_elements_scores = out_tensor.NumElements();
    batch_log_scores.clear();
    int batch = num_elements_scores / 5006; 
    batch_log_scores.resize(batch); //(batch, vector<float>(5006, 0.0));
    for(int i=0; i < batch; i++){
        batch_log_scores[i].resize(5006); 
        for(int j=0; j < 5006; j++){
            batch_log_scores[i][j] = data_scores(i*5006 + j);
        }
    }    
    return 0;
}

