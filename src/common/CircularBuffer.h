#pragma once

#include <cassert>
#include <condition_variable>
#include <deque>
#include <optional>

template<class T>
class CircularBuffer {
public:
    explicit CircularBuffer(int maxElements = 256);
    ~CircularBuffer();

public:
    [[nodiscard]]
    bool isActive() const;

    [[nodiscard]]
    bool isEmpty();

    [[nodiscard]]
    bool isFull();

    void stop();

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

    std::mutex& mutex();

    std::vector<T>& unsafe();

private:
    volatile bool  active_;
    std::mutex     mutex_;
    std::vector<T> buffer_;
    size_t         startIndex_;
    size_t         endIndex_;

    std::condition_variable cv_;
};

template<class T>
CircularBuffer<T>::CircularBuffer(int maxElements) //
        : active_(true)
        , startIndex_(0)
        , endIndex_(0) {

    buffer_.resize(maxElements);
}

template<class T>
CircularBuffer<T>::~CircularBuffer() {
    stop();
}

template<class T>
bool CircularBuffer<T>::isActive() const {
    return active_;
}

template<class T>
bool CircularBuffer<T>::isEmpty() {
    std::unique_lock lock {mutex_};

    if (!active_) {
        return true;
    }

    return startIndex_ == endIndex_;
}

template<class T>
bool CircularBuffer<T>::isFull() {
    std::unique_lock lock {mutex_};

    if (!active_) {
        return false;
    }

    const auto maxElements = buffer_.size();
    assert(maxElements != 0);

    return endIndex_ == ((startIndex_ + 1) % maxElements);
}

template<class T>
void CircularBuffer<T>::stop() {
    std::lock_guard lock {mutex_};
    active_ = false;
    cv_.notify_all();
}

template<class T>
void CircularBuffer<T>::push(const T &element) {
    std::lock_guard lock {mutex_};

    if (!active_) {
        return;
    }

    const auto maxElements = buffer_.size();
    assert(maxElements != 0);

    buffer_.at(endIndex_) = std::move(element);

    endIndex_ = (endIndex_ + 1) % maxElements;
    if (endIndex_ == startIndex_) {
        startIndex_ = (startIndex_ + 1) % maxElements;
    }

    cv_.notify_all();
}

template<class T>
std::optional<T> CircularBuffer<T>::peek() {
    std::unique_lock lock {mutex_};

    if (!active_) {
        return std::nullopt;
    }

    if (startIndex_ == endIndex_) {
        return std::nullopt;
    }

    return std::move(buffer_.at(startIndex_));
}

template<class T>
template<class Rep, class Period>
std::optional<T> CircularBuffer<T>::peek(std::chrono::duration<Rep, Period> delay) {
    std::unique_lock lock {mutex_};

    if (!active_) {
        return std::nullopt;
    }

    while (buffer_.empty()) {
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
std::optional<T> CircularBuffer<T>::pop() {
    std::unique_lock lock {mutex_};

    if (!active_) {
        return std::nullopt;
    }

    while (buffer_.empty()) {
        cv_.wait(lock);

        if (!active_) {
            return std::nullopt;
        }
    }

    auto result = std::move(buffer_.at(startIndex_));

    const auto maxElements = buffer_.size();

    startIndex_ = (startIndex_ + 1) % maxElements;

    return std::move(result);
}

template<class T>
template<class Rep, class Period>
std::optional<T> CircularBuffer<T>::pop(std::chrono::duration<Rep, Period> delay) {
    std::unique_lock lock {mutex_};

    if (!active_) {
        return std::nullopt;
    }

    while (buffer_.empty()) {
        if (cv_.wait_for(lock, delay) == std::cv_status::timeout) {
            return std::nullopt;
        }

        if (!active_) {
            return std::nullopt;
        }
    }

    auto result = std::move(buffer_.at(startIndex_));

    const auto maxElements = buffer_.size();

    startIndex_ = (startIndex_ + 1) % maxElements;

    return std::move(result);
}

template<class T>
std::mutex& CircularBuffer<T>::mutex() {
    return mutex_;
}

template<class T>
std::vector<T>& CircularBuffer<T>::unsafe() {
    return buffer_;
}
