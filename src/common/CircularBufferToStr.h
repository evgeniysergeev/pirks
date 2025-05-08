#pragma once

#include <sstream>
#include <string>

#include "CircularBuffer.h"

template<class T>
std::string CircularBufferToStr(CircularBuffer<T> &buff)
{
    using namespace std::literals;

    [[maybe_unused]]
    std::lock_guard lock { buff.mutex() };

    const auto &items = buff.unsafe();

    std::stringstream output;
    output << "{"s;
    bool       isFirst = true;
    const auto sz      = buff.bufferCapacity();
    size_t     index   = buff.startIndex();
    while (index != buff.endIndex()) {
        if (!isFirst) {
            output << ","s;
        } else {
            isFirst = false;
        }

        const auto &item = items.at(index);
        output << item;

        index = (index + 1) % sz;
    }

    output << "}"s;

    return output.str();
}
