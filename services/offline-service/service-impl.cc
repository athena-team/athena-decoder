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
#include <service-env.h>
#include "decoder-itf.h"
#include <memory>
#include <glog/logging.h>

void OfflineASRHandler(
    evpp::EventLoop* loop,
    const evpp::http::ContextPtr& ctx,
    const evpp::http::HTTPSendResponseCallback& cb){
    athena::Decoder* handler;

    int ret = ServEnv::instance()->GetHandler(handler);
    if(ret!=0){
        LOG(ERROR)<<"get handler failed";
        return;
    }
    size_t len = ctx->body().size();
    if(len == 0){
        LOG(ERROR)<<"input data len is 0";
        return;
    }
    std::shared_ptr<char> buffer(new char[len]);
    memcpy(buffer.get(),ctx->body().data(),len);

    ret = handler->PushData(buffer.get(),len,true);
    if(ret!=0){
        LOG(ERROR)<<"push data failed";
        return;
    }
    std::string trans;
    ret = handler->GetResult(trans, true);
    if(ret!=0){
        LOG(ERROR)<<"get result failed";
        return;
    }
    ret = ServEnv::instance()->GiveBackHandler(handler);
    if(ret!=0){
        LOG(ERROR)<<"give back handler failed";
        return;
    }

    std::string json = "{\"result\":\""+trans+"\"}";
    cb(json);
}
