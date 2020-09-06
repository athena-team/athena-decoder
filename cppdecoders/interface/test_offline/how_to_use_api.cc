#include <decoder-itf.h>
#include <iostream>
#include <fstream>
#include <read_pcm.h>
using namespace std;

int main(){


    athena::Resource* r=new athena::OfflineResource();
    r->LoadConfig("conf/resource.conf");
    r->CreateResource();

    athena::Decoder* d=new athena::OfflineDecoder();
    d->LoadConfig("conf/decoder.conf");
    d->CreateDecoder(r);
    //d->InitDecoder();

    ifstream infile("wav.scp",ios::in);
    ofstream outfile("results.txt",ios::out);

    while(!infile.eof()){
        string key,path;

        if(infile>>key>>path){
            short* pcm_samples = nullptr;
            int pcm_sample_count = 0;
            read_pcm_file(path.c_str(), &pcm_samples, &pcm_sample_count);
            pcm_sample_count*=2; // transfer short count to char count
            d->PushData(pcm_samples,pcm_sample_count,true);
            string result;
            d->GetResult(result);
            outfile<<key<<" "<<result<<endl;
            //d->ResetDecoder();

        }
    }
    infile.close();
    outfile.close();
    d->FreeDecoder();
    r->FreeResource();
    delete d;
    delete r;
    return 0;
}
