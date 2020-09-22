// Copyright (C) 2019 ATHENA DECODER AUTHORS; Xiangang Li; Yang Han
//  All rights reserved.
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//  ==============================================================================
#include <service-impl.h>
#include <memory>
#include <glog/logging.h>

#include <fstream>


int Context::RunDecode(const char* speech, int len, bool is_last_pkg){

    char* buffer = const_cast<char*>(speech);
    /*
    std::ofstream outFile("server.pcm",std::ios::out | std::ios::binary | std::ios::app);
    outFile.write(speech,len);
    */

    handler->PushData(buffer, len, is_last_pkg);
    std::string trans;
    handler->GetResult(trans);
    LOG(INFO)<<"ASR Result:"<<trans;
    pserver->send(hdl, trans, WS_TEXT);

    /*
    char* buffer = const_cast<char*>(speech);
    std::string str = buffer;
    std::string res = str+str;
    LOG(INFO)<<"reply:"<<res;
    sleep(10);
    pserver->send(hdl,res, WS_TEXT);
    */
    return 0;

}

std::chrono::system_clock::time_point Context::GetDeadline(){
    return deadline;
}
int Context::UpdateDeadline(std::chrono::system_clock::time_point dline){
    deadline = dline;
    return 0;
}

athena::Decoder* Context::GetHandler(){
    return handler;
}


