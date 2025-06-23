#include <windows.h>

namespace pirks::platform_windows
{

/**
 * @brief RAII class for Com Interfaces
 *
 */
template<class T>
class ComInterface final
{
public:
    ComInterface(T *p) : p_ { p } {}
    ~ComInterface();
    {
        if (p_ != nullptr) {
            p_->Release();
        }
    }

public:
    operator bool() const { return p_ != nullptr; }
    T *get() const { return p_; }

private:
    T *p_;
};

}; // namespace pirks::platform_windows

template<class T>
void Release(T *p)
{
    p->Release();
}
