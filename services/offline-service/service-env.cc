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
#include <service-env.h>

int ServEnv::SetConfig(std::string rconf, std::string dconf){
    if(rconf == "" || dconf == ""){
        LOG(ERROR)<<"rconf or dconf is NULL";
        return -1;
    }
    resource_conf = rconf;
    decoder_conf = dconf;
    resource = nullptr;
    pool.clear();
    return 0;
}


int ServEnv::CreateResource(){
    resource = new athena::OfflineResource();
    if(resource == nullptr){
        LOG(ERROR)<<"new resource failed";
        return -1;
    }
    int ret = resource->LoadConfig(resource_conf);
    if(ret!=0){
        LOG(ERROR)<<"load resource config failed";
        return -1;
    }
    ret = resource->CreateResource();
    if(ret!=0){
        LOG(ERROR)<<"create resource config failed";
        return -1;
    }
    return 0;
}

int ServEnv::CreateHandler(athena::Decoder*& handler){
    handler = nullptr;
    handler = new athena::OfflineDecoder();
    if(handler == nullptr){
        LOG(ERROR)<<"new decoder handler failed";
        return -1;
    }
    int ret = handler->LoadConfig(decoder_conf);
    if(ret!=0){
        LOG(ERROR)<<"load decoder handler config failed";
        return -1;
    }
    ret = handler->CreateDecoder(resource);
    if(ret!=0){
        LOG(ERROR)<<"create decoder handler failed";
        return -1;
    }
    return 0;
}


int ServEnv::GetHandler(athena::Decoder*& handler){
    {
        std::lock_guard<std::mutex> lg(mtx);
        if(!pool.empty()){
            handler = pool.front();
            pool.pop_front();
            return 0;
        }
    }
    int ret = CreateHandler(handler);
    if(ret!=0){
        LOG(ERROR)<<"create decoder handler failed";
        return -1;
    }
    return 0;
}

int ServEnv::GiveBackHandler(athena::Decoder*& handler){
    {
        std::lock_guard<std::mutex> lg(mtx);
        pool.push_back(handler);
        return 0;
    }
}

ServEnv::~ServEnv(){
    {
        std::lock_guard<std::mutex> lg(mtx);
        while(!pool.empty()){
            pool.front()->FreeDecoder();
            pool.pop_front();
        }
    }
    int ret = resource->FreeResource();
    if(ret!=0){
        LOG(ERROR)<<"free resource failed";
    }
}


