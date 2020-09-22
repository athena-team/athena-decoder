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
#ifndef __SINGLETON_H__
#define __SINGLETON_H__
#include <iostream>
#include <mutex>
#include <memory>

template <typename T>
class Singleton {
 public:
    static  std::shared_ptr<T> instance() {
        if(obj_instance == nullptr) {
            std::lock_guard<std::mutex> lg(mutex_);
            if(obj_instance == nullptr) {
                obj_instance = std::shared_ptr<T>(new T);

            }

        }
        assert(obj_instance != nullptr);
        return obj_instance;

    }
 protected:
    Singleton () {}
    virtual ~Singleton() {}
 private:
    Singleton (const Singleton &) ;
    Singleton & operator = (const Singleton &);
    static std::mutex mutex_;
    static std::shared_ptr<T> obj_instance;

};

template <typename T>
std::shared_ptr<T> Singleton<T>::obj_instance = nullptr ;
template <typename T>
std::mutex Singleton<T>::mutex_;

#endif
