#include <decoder-itf.h>
#include <iostream>
#include <fstream>
#include <read_pcm.h>
using namespace std;

int main(){


    athena::Resource* r=new athena::StreamingResource();
    r->LoadConfig("conf/resource.conf");
    r->CreateResource();

    athena::Decoder* d=new athena::StreamingDecoder();
    d->LoadConfig("conf/decoder.conf");
    d->CreateDecoder(r);
    d->InitDecoder();

    ifstream infile("wav.scp",ios::in);
    ofstream outfile("results.txt",ios::out);

    while(!infile.eof()){
        string key,path;

        if(infile>>key>>path){
            short* pcm_samples = nullptr;
            int short_size = 0;
            read_pcm_file(path.c_str(), &pcm_samples, &short_size);
            int BLOCK = 1600;
            while(short_size>0){
                int block_size = short_size > 1.2*BLOCK ? BLOCK : short_size;
                short_size -= block_size;
                if(short_size <= 0){
                    d->PushData(pcm_samples,2*block_size,true);
                    string result;
                    d->GetResult(result);
                    outfile<<key<<" "<<result<<endl;
                    d->ResetDecoder();
                }else{
                    d->PushData(pcm_samples,2*block_size,false);
                }
                pcm_samples += block_size;

            }
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
