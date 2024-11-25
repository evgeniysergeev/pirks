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

    void stop();

    void push(const T &element);

    [[nodiscard]]
    bool empty();

    [[nodiscard]]
    bool full();

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
    volatile bool   m_active;
    std::mutex      m_mutex;
    std::vector<T>  m_buffer;
    size_t          m_startIndex;
    size_t          m_endIndex;

    std::condition_variable m_cv;
};

template<class T>
CircularBuffer<T>::CircularBuffer(int maxElements) //
        : m_active(true)
        , m_startIndex(0)
        , m_endIndex(0) {

    m_buffer.resize(maxElements);
}

template<class T>
CircularBuffer<T>::~CircularBuffer() {
    stop();
}

template<class T>
bool CircularBuffer<T>::isActive() const {
    return m_active;
}

template<class T>
void CircularBuffer<T>::stop() {
    std::lock_guard lock {m_mutex};
    m_active = false;
    m_cv.notify_all();
}

template<class T>
void CircularBuffer<T>::push(const T &element) {
    std::lock_guard lock {m_mutex};

    if (!m_active) {
        return;
    }

    const auto maxElements = m_buffer.size();

    m_buffer.at(m_endIndex) = std::move(element);
    m_endIndex = (m_endIndex + 1) % maxElements;
    if (m_endIndex == m_startIndex) {
        m_startIndex = (m_startIndex + 1) % maxElements;
    }

    m_cv.notify_all();
}

template<class T>
bool CircularBuffer<T>::empty() {
    std::unique_lock lock {m_mutex};

    if (!m_active) {
        return true;
    }

    return m_startIndex == m_endIndex;
}

template<class T>
bool CircularBuffer<T>::full() {
    std::unique_lock lock {m_mutex};

    if (!m_active) {
        return false;
    }

    const auto maxElements = m_buffer.size();

    return m_endIndex == ((m_startIndex + 1) % maxElements);
}

template<class T>
std::optional<T> CircularBuffer<T>::peek() {
    std::unique_lock lock {m_mutex};

    if (!m_active) {
        return std::nullopt;
    }

    if (m_startIndex == m_endIndex) {
        return std::nullopt;
    }

    return std::move(m_buffer.at(m_startIndex));
}

template<class T>
template<class Rep, class Period>
std::optional<T> CircularBuffer<T>::peek(std::chrono::duration<Rep, Period> delay) {
    std::unique_lock lock {m_mutex};

    if (!m_active) {
        return std::nullopt;
    }

    while (m_buffer.empty()) {
        if (m_cv.wait_for(lock, delay) == std::cv_status::timeout) {
            return std::nullopt;
        }

        if (!m_active) {
            return std::nullopt;
        }
    }

    return std::move(m_buffer.at(m_startIndex));
}

template<class T>
std::optional<T> CircularBuffer<T>::pop() {
    std::unique_lock lock {m_mutex};

    if (!m_active) {
        return std::nullopt;
    }

    while (m_buffer.empty()) {
        m_cv.wait(lock);

        if (!m_active) {
            return std::nullopt;
        }
    }

    auto result = std::move(m_buffer.at(m_startIndex));

    const auto maxElements = m_buffer.size();
    m_startIndex = (m_startIndex + 1) % maxElements;

    return std::move(result);
}

template<class T>
template<class Rep, class Period>
std::optional<T> CircularBuffer<T>::pop(std::chrono::duration<Rep, Period> delay) {
    std::unique_lock lock {m_mutex};

    if (!m_active) {
        return std::nullopt;
    }

    while (m_buffer.empty()) {
        if (m_cv.wait_for(lock, delay) == std::cv_status::timeout) {
            return std::nullopt;
        }

        if (!m_active) {
            return std::nullopt;
        }
    }

    auto result = std::move(m_buffer.at(m_startIndex));

    const auto maxElements = m_buffer.size();
    m_startIndex = (m_startIndex + 1) % maxElements;

    return std::move(result);
}

template<class T>
std::mutex& CircularBuffer<T>::mutex() {
    return m_mutex;
}

template<class T>
std::vector<T>& CircularBuffer<T>::unsafe() {
    return m_buffer;
}
