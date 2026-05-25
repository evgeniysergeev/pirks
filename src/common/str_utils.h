#pragma once

#include <string>
#include <string_view>

using namespace std::literals;

void func();

#ifdef _WIN32
auto wideToUtf8(const std::wstring &wstr) -> std::string;
#endif
