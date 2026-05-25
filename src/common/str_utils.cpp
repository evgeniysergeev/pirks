#include "str_utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
auto wideToUtf8(const std::wstring &wstr) -> std::string
{
    if (wstr.empty()) {
        return {};
    }

    const int len = WideCharToMultiByte(
            CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    if (len <= 0) {
        return {};
    }

    std::string result(static_cast<std::string::size_type>(len), '\0');
    WideCharToMultiByte(
            CP_UTF8,
            0,
            wstr.c_str(),
            static_cast<int>(wstr.size()),
            result.data(),
            len,
            nullptr,
            nullptr);
    return result;
}
#endif
