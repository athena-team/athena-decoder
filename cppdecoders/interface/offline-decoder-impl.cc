// Copyright (C) 2019 ATHENA DECODER AUTHORS; Xiangang Li; Yang Han
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

#include <interface/decoder-itf.h>
#include <fstream>
#include <iostream>
#include <string>
#include <transformer/inference.h>
#include <transformer/decodable-am-transformer.h>
#include <utils/graph-io.h>
#include <utils/utils.h>
#include <decoder/transformer-faster-decoder.h>
#include <decoder/transformer-argmax-decoder.h>
#include <decoder/transformer-beam-search-decoder.h>



namespace athena{

int OfflineResource::LoadConfig(std::string config){

    std::ifstream infile(config.c_str(),std::ios::in);
    if(!infile.is_open()){
        std::cerr<<"Open Decoder Resources Config Failed";
        return -1;
    }
    std::string strLine,key,value;
    while(!infile.eof()){
        getline(infile,strLine);
        trim(strLine);
        if(strLine==""||strLine[0]=='#') continue;
        size_t pos = strLine.find('=');
        key = strLine.substr(0,pos);
        trim(key);
        value = strLine.substr(pos+1);
        trim(value);
        if(key == "am_config_path") am_config_path_ = value;
        if(key == "graph_path") graph_path_=value;
        if(key == "words_table_path") words_table_path_=value;
    }
    infile.close();
    std::cout<<"am config path : "<<am_config_path_<<std::endl;
    std::cout<<"graph path : "<<graph_path_<<std::endl;
    std::cout<<"words table path : "<<words_table_path_<<std::endl;

    return 0;
}

int OfflineResource::CreateResource(){
    if(graph_path_!=""){
        graph_=ReadGraph(graph_path_);
        if(graph_==NULL){
            std::cerr<<"Load Graph Failed";
            return -1;
        }
    }


    read_w_table(words_table_path_.c_str(),words_table_);
    std::cout<<"resource words table size:"<<words_table_.size()<<std::endl;

    if(inference::STATUS_OK != inference::LoadModel(am_config_path_.c_str(),am_)){
        std::cerr<<"Load AM Config Failed";
        return -1;
    }
    std::cout<<"Create Faster Resources Successfully";
    return 0;
}

int OfflineResource::FreeResource(){

    if(graph_ != NULL)
        delete static_cast<StdVectorFst*>(graph_);

    if(inference::STATUS_OK != inference::FreeModel(am_)){
        std::cerr<<"Unload AM Failed!";
        return -1;
    }
    std::cout<<"Free Faster Resources Successfully"<<std::endl;
    return 0;
}
void* OfflineResource::GetAM(){
    return am_;
}
void* OfflineResource::GetGraph(){
    return graph_;
}

int OfflineResource::GetWordsTable(std::vector<std::string>& wt){
    wt = words_table_;
    return 0;
}

/*
 * Decoder
 */

int OfflineDecoder::LoadConfig(std::string config){

    acoustic_scale_=3.0;
    allow_partial_=true;
    beam_ = 15.0;
    max_active_ = std::numeric_limits<int>::max();
    min_active_ = 20;
    graph_scale_ = 1.0;
    max_seq_len_ = 100;
    sos_ = 5005;
    eos_ = 5005;
    decoder_type_ = "argmax";

    std::ifstream infile(config.c_str(),std::ios::in);
    if(!infile.is_open()){
        std::cerr<<"Open Decoder Config Failed";
        return -1;
    }
    std::string strLine,key,value;
    while(!infile.eof()){
        getline(infile,strLine);
        trim(strLine);
        if(strLine==""||strLine[0]=='#') continue;
        size_t pos = strLine.find('=');
        key = strLine.substr(0,pos);
        trim(key);
        value = strLine.substr(pos+1);
        trim(value);
        if(key == "acoustic_scale") acoustic_scale_ = stof(value);
        if(key == "allow_partial") allow_partial_=stoi(value);
        if(key == "beam") beam_ = stof(value);
        if(key == "max_active") max_active_=stoi(value);
        if(key == "min_active") min_active_=stoi(value);
        if(key == "graph_scale") graph_scale_=stof(value);
        if(key == "sos") sos_=stoi(value);
        if(key == "eos") eos_=stoi(value);
        if(key == "max_seq_len") max_seq_len_=stoi(value);
        if(key == "decoder_type") decoder_type_ = value;
    }
    infile.close();

    std::cout<<"acoustic scale : "<<acoustic_scale_<<std::endl;
    std::cout<<"allow partial : "<<allow_partial_<<std::endl;
    std::cout<<"beam : "<<beam_<<std::endl;
    std::cout<<"max active : "<<max_active_<<std::endl;
    std::cout<<"min active : "<<min_active_<<std::endl;
    std::cout<<"graph scale : "<<graph_scale_<<std::endl;
    std::cout<<"max seq len : "<<max_seq_len_<<std::endl;
    std::cout<<"sos : "<<sos_<<std::endl;
    std::cout<<"eos : "<<eos_<<std::endl;
    std::cout<<"decoder type : "<<decoder_type_<<std::endl;

    std::cout<<"Load Faster Decoder Config Successfully"<<std::endl;
    return 0;
}

int OfflineDecoder::CreateDecoder(Resource* r){

    OfflineResource* cr = dynamic_cast<OfflineResource*>(r);

    cr->GetWordsTable(words_table_);
    std::cout<<"decoder words table size:"<<words_table_.size()<<std::endl;

    if(inference::STATUS_OK != inference::CreateHandle(cr->GetAM(),am_handler_)){
        std::cerr<<"Create AM Handler Failed!";
        return -1;
    }

    if(decoder_type_ == "wfst"){
        TransformerFasterDecoderOptions options;
        options.beam=beam_;
        options.max_active=max_active_;
        options.min_active=min_active_;
        options.max_seq_len=max_seq_len_;
        options.graph_scale=graph_scale_;
        decoder_ = new TransformerFasterDecoder(*static_cast<StdVectorFst*>(cr->GetGraph()),options);
        decodable_ = new TransformerDecodable(am_handler_, acoustic_scale_,sos_,eos_);
    }else if(decoder_type_ == "beamsearch"){
        TransformerBeamSearchDecoderOptions options;
        options.beam=beam_;
        options.max_seq_len=max_seq_len_;
        decoder_ = new TransformerBeamSearchDecoder(options);
        decodable_ = new TransformerDecodable(am_handler_, acoustic_scale_,sos_,eos_);
    }else if(decoder_type_ == "argmax"){
        decoder_ = new TransformerArgmaxDecoder(max_seq_len_, sos_, eos_);
        decodable_ = new TransformerDecodable(am_handler_, acoustic_scale_,sos_,eos_);
    }else{
        std::cerr<<"do not support decoder type : "<<decoder_type_<<std::endl;
        return -1;
    }
    std::cout<<"Create Faster Decoder Successfully"<<std::endl;
    return 0;
}
int OfflineDecoder::InitDecoder(){
    std::cout<<"Init Faster Decoder Successfully"<<std::endl;
    return 0;
}

int OfflineDecoder::ResetDecoder(){
    std::cout<<"Reset Faster Decoder Successfully"<<std::endl;
    return 0;
}
int OfflineDecoder::FreeDecoder(){

    if (inference::STATUS_OK != inference::FreeHandle(am_handler_)){
        std::cerr <<"Destory AM Handler Failed!";
        return -1;
    }
    if(decoder_){
        if(decoder_type_ == "wfst"){
            delete static_cast<TransformerFasterDecoder*>(decoder_);
        }else if(decoder_type_ == "beamsearch"){
            delete static_cast<TransformerBeamSearchDecoder*>(decoder_);
        }else if(decoder_type_ == "argmax"){
            delete static_cast<TransformerArgmaxDecoder*>(decoder_);
        }
        decoder_ = NULL;

    }
    if(decodable_){
        delete static_cast<TransformerDecodable*>(decodable_);
        decodable_ = NULL;
    }

    std::cout<<"Free Faster Decoder Successfully"<<std::endl;
    return 0;
}

int OfflineDecoder::PushData(void* data,int char_size,bool istail){

    if(data==NULL || char_size <=0){
        std::cerr<<"No Speech Data";
        return -1;
    }

    inference::Input speech_data;
    speech_data.pcm_raw=static_cast<char*>(data);
    speech_data.pcm_size = char_size;

    int status = static_cast<TransformerDecodable*>(decodable_)->GetEncoderOutput(&speech_data);
    if(status == inference::STATUS_ERROR){
        std::cerr<<"Calculate AM Scores Failed"<<std::endl;
        return -1;
    }else if(status == inference::STATUS_OK){
        if(decoder_type_ == "wfst"){
            static_cast<TransformerFasterDecoder*>(decoder_)->Decode(static_cast<TransformerDecodable*>(decodable_));
        }else if(decoder_type_ == "beamsearch"){
            static_cast<TransformerBeamSearchDecoder*>(decoder_)->Decode(static_cast<TransformerDecodable*>(decodable_));
        }else if(decoder_type_ == "argmax"){
            static_cast<TransformerArgmaxDecoder*>(decoder_)->Decode(static_cast<TransformerDecodable*>(decodable_));
        }

        std::cout<<"Push Data and Decode Successfully"<<std::endl;
        return 0;
    }else{
        std::cerr<<"Unknown Error When Calculate AM Scores"<<std::endl;
        return -1;
    }

}

int OfflineDecoder::GetResult(std::string& results,bool isfinal){

    std::vector<int> trans;
    if(decoder_type_ == "wfst"){
        static_cast<TransformerFasterDecoder*>(decoder_)->GetBestPath(trans);
    }else if(decoder_type_ == "beamsearch"){
        static_cast<TransformerBeamSearchDecoder*>(decoder_)->GetBestPath(trans);
    }else if(decoder_type_ == "argmax"){
        static_cast<TransformerArgmaxDecoder*>(decoder_)->GetBestPath(trans);
    }
    results="";
    for(int i=0;i<trans.size();i++){
        if(i==0){
            results += words_table_[trans[i]];
        }else{
            results += " " + words_table_[trans[i]];
        }
    }
    std::cout<<"Get Final Result Successfully. "<<"Result Size is : "<<trans.size()<<std::endl;
    return 0;
}


}// end of namespace api
