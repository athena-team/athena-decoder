#ifndef __DECODER_ITF__
#define __DECODER_ITF__

#include <string>
#include <vector>

namespace athena{

// interfaces for all resource
class Resource{ // global uniqueness, shared by all decoders
 public:
    // load config file including AM LM Graph
    virtual int LoadConfig(std::string config) = 0;
    // load all the resources 
    virtual int CreateResource() = 0;
    // release all the resources
    virtual int FreeResource() = 0;

    virtual void* GetAM(){return NULL;}
    virtual void* GetGraph(){return NULL;}
    virtual int GetWordsTable(std::vector<std::string>& wt){return 0;}
};
// interfaces for decoder
class Decoder{// multiple decoders<==>multiple speech stream <==> multiple threads
 public:
    // load config file for decoder
    virtual int LoadConfig(std::string config) = 0;
    // create decoder API, the framework of creating decoder
    virtual int CreateDecoder(Resource* r) = 0;
    // init decoder prepare for decoding
    virtual int InitDecoder() = 0;
    // push speech data and do recognition,
    // parameter size is the size of bytes; 
    // parameter data will be converted to short* type
    // char size will be converted to short size
    virtual int PushData(void* data,int char_size,bool istail) = 0;
    // get temp or final result, if isfinal==true, then use final probs to get
    // final results when sspeech.istail==true
    virtual int GetResult(std::string& results,bool isfinal=false) = 0;
    // reset decoder if reach the end of speech data
    // when speech.istail==true , you must call ResetDecoder() 
    virtual int ResetDecoder() = 0;
    // release decoder
    virtual int FreeDecoder() = 0;

};

/*
 * below is concrete class for different kinds of decoder
 * caller just need to call the interfaces above
 */

class StreamingResource:public Resource{
 public:
    virtual int LoadConfig(std::string config);
    virtual int CreateResource();
    virtual int FreeResource();
    virtual void* GetAM();
    virtual void* GetGraph();
    virtual int GetWordsTable(std::vector<std::string>& wt);
 private:
    std::string am_config_path_;
    std::string graph_path_;
    std::string words_table_path_;
    std::vector<std::string> words_table_;
    void* am_;
    void* graph_;
};

class StreamingDecoder:public Decoder{
 public:
    int LoadConfig(std::string config);
    int CreateDecoder(Resource* r);
    int InitDecoder();
    int PushData(void* data,int char_size,bool istail);
    int GetResult(std::string& results,bool isfinal=false);
    int ResetDecoder();
    int FreeDecoder();
 private:
    void* decoder_;
    void* am_handler_;
    void* decodable_;
    std::vector<std::string> words_table_;
    float acoustic_scale_;
    bool allow_partial_;
    float beam_;
    int max_active_;
    int min_active_;
    float minus_blank_;
    int blank_id_;
    bool ctc_prune_;
    float ctc_threshold_; 
    bool ishead_;

};



class OfflineResource:public Resource{
 public:
    virtual int LoadConfig(std::string config);
    virtual int CreateResource();
    virtual int FreeResource();
    virtual void* GetAM();
    virtual void* GetGraph();
    virtual int GetWordsTable(std::vector<std::string>& wt);
 private:
    std::string am_config_path_;
    std::string graph_path_;
    std::string words_table_path_;
    std::vector<std::string> words_table_;
    void* am_;
    void* graph_;
};

class OfflineDecoder:public Decoder{
 public:
    int LoadConfig(std::string config);
    int CreateDecoder(Resource* r);
    int InitDecoder();
    int PushData(void* data,int char_size,bool istail);
    int GetResult(std::string& results,bool isfinal=false);
    int ResetDecoder();
    int FreeDecoder();
 private:
    void* decoder_;
    void* am_handler_;
    void* decodable_;
    std::vector<std::string> words_table_;

    float acoustic_scale_;
    bool allow_partial_;
    float beam_ ;
    int max_active_;
    int min_active_;
    float graph_scale_;
    int max_seq_len_;
    int sos_;
    int eos_;
    std::string decoder_type_;

};




}// end of namespace

#endif
