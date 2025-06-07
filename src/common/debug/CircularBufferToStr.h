#pragma once

#include <sstream>
#include <string>

#include "../CircularBuffer.h"

/**
 * @brief Print content form a CircularBuffer instanse in one line
 * 
 * Can be used in tests or for debug logging
 * 
 * @tparam T 
 * @param buff      CurcularBuffer instance
 * @return std::string Result in one line. For example: { 1,2,3 }
 */
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
