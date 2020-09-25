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
#ifndef __SERVICE_IMPL_H__
#define __SERVICE_IMPL_H__
#include <chrono>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "decoder-itf.h"
#define WS_TEXT websocketpp::frame::opcode::text
#define WS_BINARY websocketpp::frame::opcode::binary

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::config::asio::message_type::ptr message_ptr;

/*
 * ready : ready to bind to a decoder
 * start : bind to a decoder
 * ing: doing asr
 * end : connection closed and to be delete
 * closed : have already be closed by client
 */
enum ContextStatus {
    CONTEXT_READY = 0,
    CONTEXT_START = 1,
    CONTEXT_ING = 2,
    CONTEXT_END = 3,
    CONTEXT_CLOSED = 4
};

class Context{
public:
    Context(std::string cid, std::chrono::system_clock::time_point dline,
            server* pserver, websocketpp::connection_hdl hdl,
            athena::Decoder* handler):
        cid(cid),
        deadline(dline),
        pserver(pserver),
        hdl(hdl),
        handler(handler){
            status = CONTEXT_READY;
        }

    int HandleMessage(message_ptr msg);
    int RunDecode(const char* speech, int len, bool is_last_pkg);
    int UpdateDeadline(std::chrono::system_clock::time_point dline);
    std::chrono::system_clock::time_point GetDeadline();
    athena::Decoder* GetHandler();
    int CloseConnection();
    ContextStatus GetStatus();
    int SetStatus(ContextStatus s);

private:
    server* pserver;
    websocketpp::connection_hdl hdl;
    std::string cid;
    std::chrono::system_clock::time_point deadline;
    athena::Decoder* handler;
    ContextStatus status;
};
typedef std::shared_ptr<Context> ContextPtr;


#endif
