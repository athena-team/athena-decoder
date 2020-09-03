// Copyright (C) 2019 ATHENA DECODER AUTHORS; Xiangang Li; Yonghu Gao
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ==============================================================================

#ifndef TYPE_H
#define TYPE_H
#include "buffer.h"
#include <tuple>
#include <vector>
#include <stdlib.h>
#include <memory>


namespace inference {

template <typename T>
using DATA_TYPE = std::tuple<std::string, std::unique_ptr<Buffer<T> >, std::vector<std::size_t> >;

template <typename T>
std::string get_name(DATA_TYPE<T>& in) {
    return std::get<0>(in);
}

template <typename T>
std::unique_ptr<Buffer<T> >& get_buffer(DATA_TYPE<T>& in) {
    return std::get<1>(in);
}

template <typename T>
std::vector<std::size_t>& get_shape(DATA_TYPE<T>& in) {
    return std::get<2>(in);
}

} //namespace inference

#endif //INCLUDE_TYPE_H
