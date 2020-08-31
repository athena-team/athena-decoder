#ifndef INCLUCD_BASE_H
#define INCLUCD_BASE_H

#include <string>
#include <vector>
#include <memory>
#include <vector>
#include <iostream>
#include <tuple>

namespace inference {

using std::ostream;
using std::vector;

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} //end namespace inference

#endif //end INCLUCD_BASE_H

