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
