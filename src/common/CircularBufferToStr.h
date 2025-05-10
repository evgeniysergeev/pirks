#pragma once

#include <sstream>
#include <string>

#include "CircularBuffer.h"

template<class T>
auto CircularBufferToStr(CircularBuffer<T> &buff) -> std::string
{
    using namespace std::literals;

    [[maybe_unused]]
    std::lock_guard lock { buff.mutex() };

    const auto &items = buff.unsafe();

    std::stringstream output;
    output << "{"sv;
    bool       isFirst = true;
    const auto sz      = buff.bufferCapacity();
    auto       index   = buff.startIndex();
    while (index != buff.endIndex()) {
        if (!isFirst) {
            output << ","sv;
        } else {
            isFirst = false;
        }

        const auto &item = items.at(index);
        output << item;

        index = (index + 1) % sz;
    }

    output << "}"sv;

    return output.str();
}
