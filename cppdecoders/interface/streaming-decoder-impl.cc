#include <interface/decoder-itf.h>
#include <fstream>
#include <iostream>
#include <string>
#include <ctc/inference.h>
#include <ctc/decodable-am-ctc.h>
#include <utils/graph-io.h>
#include <utils/utils.h>
#include <decoder/faster-decoder.h>



namespace athena{

int StreamingResource::LoadConfig(std::string config){

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
    std::cout<<"am config path : "<<am_config_path_;
    std::cout<<"graph path : "<<graph_path_;
    std::cout<<"words table path : "<<words_table_path_;

    return 0;
}

int StreamingResource::CreateResource(){
    graph_=ReadGraph(graph_path_);
    if(graph_==NULL){
        std::cerr<<"Load Graph Failed";
        return -1;
    }
    read_w_table(words_table_path_.c_str(),words_table_);
    if(inference::STATUS_OK != inference::LoadModel(am_config_path_.c_str(),am_)){
        std::cerr<<"Load AM Config Failed";
        return -1;
    }
    std::cout<<"Create Faster Resources Successfully";
    return 0;
}

int StreamingResource::FreeResource(){

    if(graph_ != NULL)
        delete static_cast<StdVectorFst*>(graph_);

    if(inference::STATUS_OK != inference::FreeModel(am_)){
        std::cerr<<"Unload AM Failed!";
        return -1;
    }
    std::cout<<"Free Faster Resources Successfully";
    return 0;
}
void* StreamingResource::GetAM(){
    return am_;
}
void* StreamingResource::GetGraph(){
    return graph_;
}

int StreamingResource::GetWordsTable(std::vector<std::string>& wt){
    words_table_ = wt;
    return 0;
}

/*
 * Decoder
 */

int StreamingDecoder::LoadConfig(std::string config){

    acoustic_scale_=3.0;
    allow_partial_=true;
    beam_ = 15.0;
    max_active_ = std::numeric_limits<int>::max();
    min_active_ = 20;
    minus_blank_=0;
    blank_id_=1;
    ctc_prune_=false;
    ctc_threshold_=0.1; 

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
        if(key == "minus_blank") minus_blank_=stof(value);
        if(key == "blank_id") blank_id_=stoi(value);
        if(key == "ctc_prune") ctc_prune_=stoi(value);
        if(key == "ctc_threshold") ctc_threshold_=stof(value);
        if(key == "acoustic_scale") acoustic_scale_ = stof(value);
        if(key == "allow_partial") allow_partial_=stoi(value);
        if(key == "beam") beam_ = stof(value);
        if(key == "max_active") max_active_=stoi(value);
        if(key == "min_active") min_active_=stoi(value);
    }
    infile.close();

    std::cout<<"acoustic scale : "<<acoustic_scale_;
    std::cout<<"allow partial : "<<allow_partial_;
    std::cout<<"beam : "<<beam_;
    std::cout<<"max active : "<<max_active_;
    std::cout<<"min active : "<<min_active_;
    std::cout<<"minus blank : "<<minus_blank_;
    std::cout<<"blank id : "<<blank_id_;
    std::cout<<"ctc prune : "<<ctc_prune_;
    std::cout<<"ctc threshold : "<<ctc_threshold_;
    std::cout<<"Load Faster Decoder Config Successfully";
    return 0;
}

int StreamingDecoder::CreateDecoder(Resource* r){

    StreamingResource* cr = dynamic_cast<StreamingResource*>(r);
    cr->GetWordsTable(words_table_);

    if(inference::STATUS_OK != inference::CreateHandle(cr->GetAM(),am_handler_)){
        std::cerr<<"Create AM Handler Failed!";
        return -1;
    }

    FasterDecoderOptions options;
    options.beam=beam_;
    options.max_active=max_active_;
    options.min_active=min_active_;

    decoder_ = new FasterDecoder(*static_cast<StdVectorFst*>(cr->GetGraph()),options);
    decodable_ = new DecodableCTC(am_handler_, acoustic_scale_,blank_id_,minus_blank_,ctc_prune_,ctc_threshold_,0,NULL);

    std::cout<<"Create Faster Decoder Successfully";
    return 0;
}
int StreamingDecoder::InitDecoder(){
    static_cast<FasterDecoder*>(decoder_)->InitDecoding();
    static_cast<DecodableCTC*>(decodable_)->Reset();
    ishead_ = true;
    std::cout<<"Init Faster Decoder Successfully";
    return 0;
}

int StreamingDecoder::ResetDecoder(){
    static_cast<FasterDecoder*>(decoder_)->InitDecoding();
    static_cast<DecodableCTC*>(decodable_)->Reset();
    ishead_ = true;
    std::cout<<"Reset Faster Decoder Successfully";
    return 0;
}
int StreamingDecoder::FreeDecoder(){

    if (inference::STATUS_OK != inference::FreeHandle(am_handler_)){
        std::cerr <<"Destory AM Handler Failed!";
        return -1;
    }
    if(decoder_){
        delete static_cast<FasterDecoder*>(decoder_);
        decoder_ = NULL;

    }
    if(decodable_){
        delete static_cast<DecodableCTC*>(decodable_);
        decodable_ = NULL;
    }

    std::cout<<"Free Faster Decoder Successfully";
    return 0;
}

int StreamingDecoder::PushData(void* data,int short_size,bool istail){

    if(data==NULL || short_size <=0){
        std::cerr<<"No Speech Data";
        return -1;
    }

    inference::Input speech_data;
    speech_data.pcm_raw=static_cast<char*>(data);
    speech_data.pcm_size = short_size*2;

    speech_data.first=ishead_;
    speech_data.last=istail;

    if(ishead_ || istail){
        if(ishead_){ 
            std::cout<<"the First Data Block";
            ishead_=false;
        }
        if(istail){
            std::cout<<"the Last Data Block";
        }
    }
    else{
        std::cout<<"the Middle Data Block";
    }

    int status = static_cast<DecodableCTC*>(decodable_)->GetEncoderOutput(&speech_data);
    if(status == inference::STATUS_ERROR){
        std::cerr<<"Calculate AM Scores Failed";
        return -1;
    }else if(status == inference::STATUS_OK){
        static_cast<FasterDecoder*>(decoder_)->AdvanceDecoding(static_cast<DecodableCTC*>(decodable_));
        std::cout<<"Push Data and Decode Successfully";
        return 0;
    }else{
        std::cerr<<"Unknown Error When Calculate AM Scores";
        return -1;
    }

}

int StreamingDecoder::GetResult(std::string& results,bool isfinal){

    std::vector<int> trans;
    static_cast<FasterDecoder*>(decoder_)->GetBestPath(trans);
    results="";
    for(int i=0;i<trans.size();i++){
        if(i==0){
            results += words_table_[trans[i]];
        }else{
            results += " " + words_table_[trans[i]];
        }
    }

    if(isfinal)
        std::cout<<"Get Final Result Successfully. "<<"Result Size is : "<<trans.size();
    else
        std::cout<<"Get Temp Result Successfully. "<<"Result Size is : "<<trans.size();

    return 0;
}


}// end of namespace api
