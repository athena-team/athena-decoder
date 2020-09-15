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
#ifndef __SERVICE_ENV_H__
#define __SERVICE_ENV_H__

#include "singleton.h"
#include <mutex>
#include "decoder-itf.h"
#include <deque>
#include "glog/logging.h"

class ServEnv:public Singleton<ServEnv>{
public:
    int SetConfig(std::string rconf, std::string dconf);
    int CreateResource();
    int GetHandler(athena::Decoder*& handler);
    int GiveBackHandler(athena::Decoder*& handler);
    ~ServEnv();
private:
    int CreateHandler(athena::Decoder*& handler);
    std::deque<athena::Decoder*> pool;
    std::mutex mtx;
    athena::Resource* resource;
    std::string resource_conf;
    std::string decoder_conf;
};
#endif
