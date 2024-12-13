#pragma once

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <optional>

template<class T>
class CircularBuffer
{
public:
    explicit CircularBuffer(size_t capacity = 256);
    ~CircularBuffer();

public:
    void push(const T &element);

    [[nodiscard]]
    std::optional<T> peek();

    template<class Rep, class Period>
    [[nodiscard]]
    std::optional<T> peek(std::chrono::duration<Rep, Period> delay);

    [[nodiscard]]
    std::optional<T> pop();

    template<class Rep, class Period>
    [[nodiscard]]
    std::optional<T> pop(std::chrono::duration<Rep, Period> delay);

    // Helpers
public:
    [[nodiscard]]
    bool isActive() const;

    [[nodiscard]]
    bool isEmpty();

    [[nodiscard]]
    bool isFull();

    [[nodiscard]]
    size_t capacity() const;

    void stop();

    // Unsafe access to buffer. Useful for unit tests. Use with caution.
public:
    std::mutex &mutex();

    std::vector<T> &unsafe();

    [[nodiscard]]
    size_t startIndex() const;

    [[nodiscard]]
    size_t endIndex() const;

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
    //  \ start and are now equal
    //
    // Move start one element forward so start and end will be different:
    // | _ | 2 | 3 | 4 |
    //  ^    ^
    //  |    \ start
    //  \ end
    //
    // One element is always unused.
    buffer_.resize(capacity);
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

    const auto capacity = buffer_.capacity();
    assert(capacity != 0);

    buffer_.at(endIndex_) = std::move(element);

    endIndex_ = (endIndex_ + 1) % capacity;
    if (endIndex_ == startIndex_) {
        startIndex_ = (startIndex_ + 1) % capacity;
    }

    cv_.notify_all();
}

template<class T>
std::optional<T> CircularBuffer<T>::peek()
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
std::optional<T> CircularBuffer<T>::peek(std::chrono::duration<Rep, Period> delay)
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
std::optional<T> CircularBuffer<T>::pop()
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

    const auto capacity = buffer_.capacity();

    startIndex_ = (startIndex_ + 1) % capacity;

    return std::move(result);
}

template<class T>
template<class Rep, class Period>
std::optional<T> CircularBuffer<T>::pop(std::chrono::duration<Rep, Period> delay)
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

    const auto capacity = buffer_.capacity();

    startIndex_ = (startIndex_ + 1) % capacity;

    return std::move(result);
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

    const auto capacity = buffer_.capacity();
    assert(capacity != 0);

    return endIndex_ == ((startIndex_ + 1) % capacity);
}

template<class T>
size_t CircularBuffer<T>::capacity() const
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
std::mutex &CircularBuffer<T>::mutex()
{
    return mutex_;
}

template<class T>
std::vector<T> &CircularBuffer<T>::unsafe()
{
    return buffer_;
}

template<class T>
size_t CircularBuffer<T>::startIndex() const
{
    return startIndex_;
}

template<class T>
size_t CircularBuffer<T>::endIndex() const
{
    return endIndex_;
}
