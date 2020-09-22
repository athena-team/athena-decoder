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
    expire_sec = 60; // expire time 60s
    return 0;
}


int ServEnv::CreateResource(){
    resource = new athena::StreamingResource();
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

int ServEnv::CreateHandlers(int thread_num){

    std::lock_guard<std::mutex> lg(mtx);
    for(int i=0;i<thread_num;i++){
        athena::Decoder* handler = new athena::StreamingDecoder();
        handler->LoadConfig(decoder_conf);
        handler->CreateDecoder(resource);
        handler->InitDecoder();
        pool.push_back(handler);
    }
    return 0;
}


int ServEnv::GetHandler(athena::Decoder*& handler){

    std::lock_guard<std::mutex> lg(mtx);
    if(!pool.empty()){
        handler = pool.front();
        pool.pop_front();
        return 0;
    }else{
        LOG(ERROR)<<"get decoder handler failed";
        return -1;
    }
}

int ServEnv::GiveBackHandler(athena::Decoder* handler){

    handler->ResetDecoder();
    std::lock_guard<std::mutex> lg(mtx);
    pool.push_back(handler);
    handler = nullptr;
    return 0;
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


int ServEnv::GetContext(std::string& cid, server* pserver, 
        websocketpp::connection_hdl hdl,
        ContextPtr& cptr){
    //std::lock_guard<std::mutex> lg(mtx);
    mtx.lock();
    auto itr = contexts.find(cid);
    if(itr == contexts.end()){
        LOG(INFO)<<"create new context";
        mtx.unlock();
        int ret = CreateContext(cid, pserver, hdl);
        if(ret!=0){
            LOG(INFO)<<"create new context failed";
            return -1;
        }
        itr = contexts.find(cid);
    }

    if(itr == contexts.end()){
        LOG(ERROR)<<"get context failed";
        mtx.unlock();
        return -1;
    }

    cptr = itr->second;
    cptr->UpdateDeadline(std::chrono::system_clock::now()+
            std::chrono::seconds(expire_sec));

    mtx.unlock();
    return 0;
}

int ServEnv::CreateContext(std::string& cid, server* pserver,
        websocketpp::connection_hdl hdl){
    {
        std::lock_guard<std::mutex> lg(mtx);
        auto itr = contexts.find(cid);
        if(itr != contexts.end()){
            LOG(ERROR)<<"connection context already exists";
            return -1;
        }
    }

    athena::Decoder* handler = nullptr;
    int ret = GetHandler(handler);
    if(ret!=0){
        LOG(ERROR)<<"get decoder handler failed";
        return -1;
    }

    ContextPtr cptr(new Context(cid, 
                std::chrono::system_clock::now()+std::chrono::seconds(expire_sec),
                pserver, hdl, handler));
    {
        std::lock_guard<std::mutex> lg(mtx);
        contexts.insert(std::pair<std::string, ContextPtr>(cid, cptr));
        return 0;
    }
}

int ServEnv::DeleteContext(std::string& cid){

    GiveBackHandler(contexts[cid]->GetHandler());
    std::lock_guard<std::mutex> lg(mtx);
    contexts.erase(cid);
    return 0;
}

int ServEnv::DeleteContext(){
    std::vector<std::string> del_con_vec;
    {
        std::lock_guard<std::mutex> lg(mtx);
        auto itr = contexts.begin();
        for(;itr!=contexts.end();itr++){
            if(itr->second->GetDeadline() > std::chrono::system_clock::now()){
                del_con_vec.push_back(itr->first);
            }
        }

    }

    {
        std::lock_guard<std::mutex> lg(mtx);
        auto itr = del_con_vec.begin();
        for(;itr!=del_con_vec.end();itr++){
            GiveBackHandler(contexts[*itr]->GetHandler());
            contexts.erase(*itr);
        }
    }
    return 0;
}




