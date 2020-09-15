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
#include <evpp/http/http_server.h>
#include <glog/logging.h>
#include <evpp/event_loop.h>
#include "service-impl.h"
#include "service-env.h"

void DefaultHandler(evpp::EventLoop* loop,
                    const evpp::http::ContextPtr& ctx,
                    const evpp::http::HTTPSendResponseCallback& cb) {
    std::stringstream oss;
    oss << "func=" << __FUNCTION__ << " OK"
        << " ip=" << ctx->remote_ip() << "\n"
        << " uri=" << ctx->uri() << "\n"
        << " body=" << ctx->body().ToString() << "\n";
    ctx->AddResponseHeader("Content-Type", "application/octet-stream");
    ctx->AddResponseHeader("Server", "evpp");
    cb(oss.str());
}

void TimerOutHandler() {
    LOG(INFO) << "TimerOutTask::handle triggered";
    return;
}


int main(int argc, char* argv[]) {
    
    int ret = ServEnv::instance()->SetConfig("conf/resource.conf","conf/decoder.conf");
    if(ret!=0){
        LOG(ERROR)<<"set server env config failed";
    }else{
        LOG(INFO)<<"set server env config successfully";
    }
    ret = ServEnv::instance()->CreateResource();
    if(ret!=0){
        LOG(ERROR)<<"create env resource failed";
    }else{
        LOG(INFO)<<"create env resource successfully";
    }

    std::vector<int> ports{5063};
    int thread_num = 2;
    evpp::http::Server server(thread_num);
    server.SetThreadDispatchPolicy(evpp::ThreadDispatchPolicy::kRoundRobin);
    server.RegisterDefaultHandler(&DefaultHandler);
    server.RegisterHandler("/offlineasr",&OfflineASRHandler);
    server.Init(ports);
    server.Start();
    LOG(INFO)<<"ready to receive request";
    LOG(INFO)<<"e.g:curl \"localhost:5063/offlineasr\" -X POST -T test1.pcm";

    evpp::EventLoop loop;
    loop.RunEvery(evpp::Duration(static_cast<float>(1200)), &TimerOutHandler);  
    loop.Run();

    return 0;
}
