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
#include "service-env.h"
#include <memory>
#include <glog/logging.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int Context::HandleMessage(message_ptr msg){
    if(WS_TEXT==msg->get_opcode()){
        json params = json::parse(msg->get_payload());
        if("START" == params["opt"]){
            if(status != CONTEXT_READY){
                LOG(ERROR)<<"previous status should be ready, but "<<status;
                return -1;
            }
            status = CONTEXT_START;
            LOG(INFO)<<"status:"<<status;
        }
        if("STOP" == params["opt"]){
            if(status != CONTEXT_START && status != CONTEXT_ING){
                LOG(ERROR)<<"previous status should be start or ing, but "<<status;
                return -1;
            }
            status = CONTEXT_END;
            LOG(INFO)<<"status:"<<status;
            ServEnv::instance()->DeleteContext();
        }

    }else if(WS_BINARY==msg->get_opcode()){
        if(status != CONTEXT_START && status != CONTEXT_ING){
            LOG(ERROR)<<"previous status should be start or ing, but "<<status;
            return -1;
        }
        status = CONTEXT_ING;
        LOG(INFO)<<"status:"<<status;
        std::string speech = msg->get_payload();
        return RunDecode(speech.c_str(), speech.size(),false);
    }else{
        LOG(ERROR)<<"message type error";
        return -1;
    }
}


int Context::RunDecode(const char* speech, int len, bool is_last_pkg){

    char* buffer = const_cast<char*>(speech);
    int ret = handler->PushData(buffer, len, is_last_pkg);
    if(ret != 0){
        LOG(INFO)<<"Push Data Failed";
        return -1;
    }
    std::string trans;
    ret = handler->GetResult(trans);
    if(ret !=0){
        LOG(INFO)<<"Get Result Failed";
        return -1;
    }
    json result;
    result["result"] = trans;
    pserver->send(hdl, result.dump(), WS_TEXT);
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

int Context::CloseConnection(){
    pserver->close(hdl, websocketpp::close::status::going_away, "");
    return 0;
}
ContextStatus Context::GetStatus(){
    return status;
}

int Context::SetStatus(ContextStatus s){
    status = s;
    return 0;
}

