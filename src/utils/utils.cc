#include <utils/utils.h>

std::string& trim(std::string& s){
    if(s.empty()) return s;
    if(s.find_first_of("#")!=-1)
        s.erase(s.find_first_of("#"));
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ")+1);
    return s;
}

int ReadPriorLogScores(std::string prior_rxfilename, float** prior_log_scores){
    if(prior_rxfilename == "" ||
            prior_log_scores == NULL){
        std::cerr<<"param error";
        return -1;
    }
    std::ifstream prior_file(prior_rxfilename.c_str(),std::ios::in);
    if(!prior_file.is_open()){
        std::cerr<<"open prior file error";
        return -1;
    }
    float score;
    int i = 0;
    while(!prior_file.eof()){
        prior_file >> score;
        (*prior_log_scores)[i] = score;
        i++;
    }
    prior_file.close();
    std::cout<<"read prior scores successfully";
    std::cout<<"prior scores dim : "<<i-1;
    return 0;
}


std::ostream& WriteTrans(std::ostream& strm,std::string key,std::vector<int>& trans){
    strm<<key<<" ";
    for(int i=0;i<trans.size();i++){
        strm<<trans[i]<<" ";
    }
    strm<<std::endl;
    return strm;
}

bool read_w_table(const char* wtablefile, std::vector<std::string>& table){

    table.clear();

    if(wtablefile==NULL) return false;
    std::ifstream in(wtablefile,std::ios::in);
    if(!in.is_open()){
        std::cerr<<"open file failed";
        return false;
    }
    std::string word;
    int id;
    while(!in.eof()){
        in>>word>>id;
        table.push_back(word);
    }
    return true;
}

int ReadPCMFile(const char* pcm_file, short** pcm_samples, int* pcm_sample_count) {
    FILE* fpFrom = fopen(pcm_file, "rb");
    if (!fpFrom) {
        std::cerr << "Failed open pcm file " << pcm_file << std::endl;
        return -1;
    }
    fseek(fpFrom, 0, SEEK_END);
    int pcm_bytes = ftell(fpFrom);
    if(*pcm_samples){
        free(*pcm_samples);
        *pcm_samples = NULL;
    }
    *pcm_samples = static_cast<short*>(malloc(pcm_bytes));
    rewind(fpFrom);
    *pcm_sample_count = pcm_bytes / sizeof(short);
    fread(*pcm_samples, sizeof(short), *pcm_sample_count, fpFrom);
    fclose(fpFrom);
    return 0;
}


