#ifndef BUFFER_H
#define BUFFER_H
#include <stdlib.h>
#include <iostream>
#include <cstring>

namespace inference {

template<typename T>
class Buffer {
public:
    Buffer():_size(0), _ptr(nullptr) {}

    Buffer(const size_t size):
        _size(size),
        _ptr(nullptr) {
        if (_size > 0) {
            _ptr = new T[_size];
        }
    }

    ~Buffer() {
        delete [] _ptr;
    }

    size_t resize(const size_t size) {
        if (!size) {
            std::cerr << "Warning, size is 0";
            return 0;
        }

        if (!_ptr) {
            _ptr = new T[size];
        }

        if (size > _size) {
            delete [] _ptr;
            _size = size;
            _ptr = new T[_size];
        }

        return _size;
    }

    void free() {
        if (_ptr != nullptr) {
            delete [] _ptr;
            _ptr = nullptr;
            _size = 0;
        }
    }

    T* ptr() {
        return _ptr;
    }

    T& operator[](int i) {
        return *(_ptr + i);
    }

    const T operator[](int i) const {
        return *(_ptr + i);
    }

    size_t size() {
        return _size;
    }

    void zero() {
        if (nullptr != _ptr) {
            std::memset(_ptr, 0, _size * sizeof(T));
        }
    }

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

private:
    size_t _size; //当前的有效数据长度, 单位是类型T
    T* _ptr;
};

}



#endif
