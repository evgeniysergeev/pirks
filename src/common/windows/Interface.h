#pragma once

#include <windows.h>

namespace pirks::platform_windows
{

/**
 * @brief RAII class for Com Interfaces
 *
 */
template<typename T>
class Interface
{
public:
    Interface(T *p = nullptr) : pointer_ { p } {}

    virtual ~Interface()
    {
        if (pointer_ != nullptr) {
            pointer_->Release();
        }
    }

public:
    T *get() const { return pointer_; }

       operator bool() const { return pointer_ != nullptr; }
    T *operator->() const { return pointer_; }
    T &operator*() const { return *pointer_; }

protected:
    T *pointer_ { nullptr };
};
}; // namespace pirks::platform_windows
