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
