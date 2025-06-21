#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <optional>
#include <span>
#include <vector>

template<class T>
class CircularBuffer
{
public:
    explicit CircularBuffer(size_t capacity = 256);
    ~CircularBuffer();

public:
    void push(const T &element);

    // The same as above, but with only one mutex lock for all elements
    void push(std::span<const T> elements);

    // Read element but do not remove it from buffer
    [[nodiscard]]
    auto peek() -> std::optional<T>;

    template<class Rep, class Period>
    [[nodiscard]]
    auto peek(std::chrono::duration<Rep, Period> delay) -> std::optional<T>;

    [[nodiscard]]
    auto pop() -> std::optional<T>;

    [[nodiscard]]
    auto pop(std::vector<T> &out_buffer, size_t max_elements) -> size_t;

    template<class Rep, class Period>
    [[nodiscard]]
    auto pop(std::chrono::duration<Rep, Period> delay) -> std::optional<T>;

    template<class Rep, class Period>
    [[nodiscard]]
    auto pop(
            std::vector<T>                    &out_buffer,
            size_t                             max_elements,
            std::chrono::duration<Rep, Period> delay) -> size_t;

    // Helpers
public:
    [[nodiscard]]
    bool isActive() const;

    [[nodiscard]]
    bool isEmpty();

    [[nodiscard]]
    bool isFull();

    [[nodiscard]]
    auto bufferCapacity() const -> size_t;

    void stop();

    // Unsafe access to buffer. Useful for unit tests. Use with caution.
public:
    auto mutex() -> std::mutex &;

    auto unsafe() -> std::vector<T> &;

    [[nodiscard]]
    auto startIndex() const -> size_t;

    [[nodiscard]]
    auto endIndex() const -> size_t;

private:
    volatile bool  active_;
    std::mutex     mutex_;
    std::vector<T> buffer_;
    size_t         startIndex_;
    size_t         endIndex_;

    std::condition_variable cv_;
};

template<class T>
CircularBuffer<T>::CircularBuffer(size_t capacity) //
        : active_(true)
        , startIndex_(0)
        , endIndex_(0)
{
    // If start equals end, it means the buffer is empty.
    // But in a completely full buffer, at some point end will equal start:
    // | 1 | 2 | 3 | _ |
    //  ^            ^
    //  \ start      \ end
    //
    // Adding new element: 4 (end = (end + 1) % 4
    // | 1 | 2 | 3 | 4 |
    //  ^
    //  \ start
    //      and
    //  ^
    //   \ end are now equal
    //
    // Move start one element forward so start and end will be different:
    // | _ | 2 | 3 | 4 |
    //  ^    ^
    //  |    \ start
    //  \ end
    //
    // One element is always unused.
    buffer_.resize(capacity + 1);
}

template<class T>
CircularBuffer<T>::~CircularBuffer()
{
    stop();
}

template<class T>
void CircularBuffer<T>::push(const T &element)
{
    std::lock_guard lock { mutex_ };

    if (!active_) {
        return;
    }

    const auto sz = buffer_.capacity();
    assert(sz != 0);

    buffer_.at(endIndex_) = std::move(element);

    endIndex_ = (endIndex_ + 1) % sz;
    if (endIndex_ == startIndex_) {
        startIndex_ = (startIndex_ + 1) % sz;
    }

    cv_.notify_all();
}

template<class T>
void CircularBuffer<T>::push(std::span<const T> elements)
{
    std::lock_guard lock { mutex_ };

    if (!active_) {
        return;
    }

    const auto sz = buffer_.capacity();
    assert(sz != 0);

    // TODO: Should we check situation when number of elements
    //       is greater than the buffer size?
    for (const auto &element : elements) {
        buffer_.at(endIndex_) = std::move(element);

        endIndex_ = (endIndex_ + 1) % sz;
        if (endIndex_ == startIndex_) {
            startIndex_ = (startIndex_ + 1) % sz;
        }
    }

    cv_.notify_all();
}

template<class T>
auto CircularBuffer<T>::peek() -> std::optional<T>
{
    std::unique_lock lock { mutex_ };

    if (!active_) {
        return std::nullopt;
    }

    // if start index equal to end index, then buffer is empty
    if (startIndex_ == endIndex_) {
        return std::nullopt;
    }

    return std::move(buffer_.at(startIndex_));
}

template<class T>
template<class Rep, class Period>
auto CircularBuffer<T>::peek(std::chrono::duration<Rep, Period> delay) -> std::optional<T>
{
    std::unique_lock lock { mutex_ };

    if (!active_) {
        return std::nullopt;
    }

    // if start index equal to end index, then buffer is empty
    while (startIndex_ == endIndex_) {
        if (cv_.wait_for(lock, delay) == std::cv_status::timeout) {
            return std::nullopt;
        }

        if (!active_) {
            return std::nullopt;
        }
    }

    return std::move(buffer_.at(startIndex_));
}

template<class T>
auto CircularBuffer<T>::pop() -> std::optional<T>
{
    std::unique_lock lock { mutex_ };

    if (!active_) {
        return std::nullopt;
    }

    // if start index is equal to end index, then buffer is empty
    while (startIndex_ == endIndex_) {
        cv_.wait(lock);

        if (!active_) {
            return std::nullopt;
        }
    }

    auto result = std::move(buffer_.at(startIndex_));

    const auto sz = buffer_.capacity();

    startIndex_ = (startIndex_ + 1) % sz;

    return result;
}

template<class T>
[[nodiscard]]
auto CircularBuffer<T>::pop(std::vector<T> &out_buffer, size_t max_elements) -> size_t
{
    std::unique_lock lock { mutex_ };

    if (!active_) {
        return 0;
    }

    // if start index is equal to end index, then buffer is empty
    while (startIndex_ == endIndex_) {
        cv_.wait(lock);

        if (!active_) {
            return 0;
        }
    }

    const auto sz = buffer_.capacity();
    out_buffer.resize(std::min(sz, max_elements));
    size_t i = 0;
    while (i < max_elements) {
        out_buffer[i++] = std::move(buffer_.at(startIndex_));
        startIndex_     = (startIndex_ + 1) % sz;
        if (startIndex_ == endIndex_) {
            break;
        }
    }

    return i;
}

template<class T>
template<class Rep, class Period>
auto CircularBuffer<T>::pop(std::chrono::duration<Rep, Period> delay) -> std::optional<T>
{
    std::unique_lock lock { mutex_ };

    if (!active_) {
        return std::nullopt;
    }

    // if start index equal to end index, then buffer is empty
    while (startIndex_ == endIndex_) {
        if (cv_.wait_for(lock, delay) == std::cv_status::timeout) {
            return std::nullopt;
        }

        if (!active_) {
            return std::nullopt;
        }
    }

    auto result = std::move(buffer_.at(startIndex_));

    const auto sz = buffer_.capacity();

    startIndex_ = (startIndex_ + 1) % sz;

    return result;
}

template<class T>
template<class Rep, class Period>
auto CircularBuffer<T>::pop(
        std::vector<T>                    &out_buffer,
        size_t                             max_elements,
        std::chrono::duration<Rep, Period> delay) -> size_t
{
    std::unique_lock lock { mutex_ };

    if (!active_) {
        return 0;
    }

    // if start index equal to end index, then buffer is empty
    while (startIndex_ == endIndex_) {
        if (cv_.wait_for(lock, delay) == std::cv_status::timeout) {
            return 0;
        }

        if (!active_) {
            return 0;
        }
    }

    const auto sz = buffer_.capacity();
    out_buffer.resize(std::min(sz, max_elements));
    size_t i = 0;
    while (i < max_elements) {
        out_buffer[i++] = std::move(buffer_.at(startIndex_));
        startIndex_     = (startIndex_ + 1) % sz;
        if (startIndex_ == endIndex_) {
            break;
        }
    }

    return i;
}

// Helpers

template<class T>
bool CircularBuffer<T>::isActive() const
{
    return active_;
}

template<class T>
bool CircularBuffer<T>::isEmpty()
{
    std::unique_lock lock { mutex_ };

    if (!active_) {
        return true;
    }

    return startIndex_ == endIndex_;
}

template<class T>
bool CircularBuffer<T>::isFull()
{
    std::unique_lock lock { mutex_ };

    if (!active_) {
        return false;
    }

    const auto sz = buffer_.capacity();
    assert(sz != 0);

    return endIndex_ == ((startIndex_ + 1) % sz);
}

template<class T>
auto CircularBuffer<T>::bufferCapacity() const -> size_t
{
    return buffer_.capacity();
}

template<class T>
void CircularBuffer<T>::stop()
{
    std::lock_guard lock { mutex_ };
    active_ = false;
    cv_.notify_all();
}

// Unsafe access to buffer. Useful for unit tests. Use with caution.

template<class T>
auto CircularBuffer<T>::mutex() -> std::mutex &
{
    return mutex_;
}

template<class T>
auto CircularBuffer<T>::unsafe() -> std::vector<T> &
{
    return buffer_;
}

template<class T>
auto CircularBuffer<T>::startIndex() const -> size_t
{
    return startIndex_;
}

template<class T>
auto CircularBuffer<T>::endIndex() const -> size_t
{
    return endIndex_;
}
