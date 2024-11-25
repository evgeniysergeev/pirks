#pragma once

#include <condition_variable>
#include <deque>

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
    std::optional<T> peek();

    template<class Rep, class Period>
    [[nodiscard]]
    std::optional<T> peek(std::chrono::duration<Rep, Period> delay);

    [[nodiscard]]
    std::optional<T> pop();

    template<class Rep, class Period>
    [[nodiscard]]
    std::optional<T> pop(std::chrono::duration<Rep, Period> delay);

private:
    volatile bool m_active;
    size_t        m_maxElements;

    std::mutex    m_mutex;
    std::deque<T> m_buffer;

    std::condition_variable m_cv;
};

template<class T>
CircularBuffer<T>::CircularBuffer(int maxElements) //
        : m_active(true)
        , m_maxElements(maxElements) {
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

    if (m_buffer.size() == m_maxElements) {
        m_buffer.pop_front();
    }

    m_buffer.emplace_back(std::move(element));

    m_cv.notify_all();
}

template<class T>
std::optional<T> CircularBuffer<T>::peek() {
    std::unique_lock lock {m_mutex};

    if (!m_active) {
        return std::nullopt;
    }

    if (m_buffer.empty()) {
        return std::nullopt;
    }

    return m_buffer.front();
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

    return m_buffer.front();
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

    return m_buffer.pop_front();
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

    return m_buffer.pop_front();
}
