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
    {
        std::lock_guard<std::mutex> lg(pmtx);
        pool.clear();
    }
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

    std::lock_guard<std::mutex> lg(pmtx);
    for(int i=0;i<thread_num;i++){
        athena::Decoder* handler = new athena::StreamingDecoder();
        handler->LoadConfig(decoder_conf);
        handler->CreateDecoder(resource);
        handler->InitDecoder();
        pool.push_back(handler);

        ThreadPoolPtr th = std::shared_ptr<ThreadPool>(new ThreadPool(1));
        task_runner[handler] = th;
    }
    return 0;
}

ThreadPoolPtr ServEnv::GetThreadPool(athena::Decoder* handler){
    return task_runner[handler];
}

int ServEnv::GetHandler(athena::Decoder*& handler){

    std::lock_guard<std::mutex> lg(pmtx);
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
    std::lock_guard<std::mutex> lg(pmtx);
    pool.push_back(handler);
    handler = nullptr;
    return 0;
}

ServEnv::~ServEnv(){
    {
        std::lock_guard<std::mutex> lg(pmtx);
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

bool ServEnv::DetectContext(std::string& cid){
    std::lock_guard<std::mutex> lg(cmtx);
    auto itr = contexts.find(cid);
    if(itr == contexts.end()){
        return false;
    }else{
        return true;
    }
}


int ServEnv::GetContext(std::string& cid, ContextPtr& cptr){

    std::lock_guard<std::mutex> lg(cmtx);
    auto itr = contexts.find(cid);
    if(itr == contexts.end()){
        LOG(ERROR)<<"context do not exist";
        return -1;
    }
    cptr = itr->second;
    CHECK(cptr->GetStatus() != CONTEXT_CLOSED);
    auto new_deadline = std::chrono::system_clock::now() + std::chrono::seconds(expire_sec);
    cptr->UpdateDeadline(new_deadline);
    return 0;
}

int ServEnv::CreateContext(std::string& cid, server* pserver,
        websocketpp::connection_hdl hdl){
    {
        std::lock_guard<std::mutex> lg(cmtx);
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
        std::lock_guard<std::mutex> lg(cmtx);
        contexts.insert(std::pair<std::string, ContextPtr>(cid, cptr));
        return 0;
    }
}

int ServEnv::DeleteContext(){
    std::vector<std::string> del_con_vec;
    {
        std::lock_guard<std::mutex> lg(cmtx);
        auto itr = contexts.begin();
        for(;itr!=contexts.end();itr++){

            if(itr->second->GetStatus() == CONTEXT_CLOSED ||
                    itr->second->GetStatus() == CONTEXT_END){
                // have be closed by client
                del_con_vec.push_back(itr->first);

            } else if(itr->second->GetDeadline() < std::chrono::system_clock::now()){
                // time out
                itr->second->SetStatus(CONTEXT_END);
                del_con_vec.push_back(itr->first);
            }

        }

    }

    {
        std::lock_guard<std::mutex> lg(cmtx);
        auto itr = del_con_vec.begin();
        for(;itr!=del_con_vec.end();itr++){
            GiveBackHandler(contexts[*itr]->GetHandler());
            CHECK(contexts[*itr]->GetStatus() == CONTEXT_END ||
                    contexts[*itr]->GetStatus() == CONTEXT_CLOSED);
            // if connection have already be closed by client,
            // server could not close the same connection twice
            // cause segmentation fault
            if(contexts[*itr]->GetStatus() == CONTEXT_END){
                contexts[*itr]->CloseConnection();
            }
            contexts.erase(*itr);
        }
    }
    return 0;
}




