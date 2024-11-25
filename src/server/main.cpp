#include <iostream>

#include "str_utils.h"
#include "version.h"

int main() {
    std::cout << PROJECT_NAME << " v"sv << PROJECT_FULL_VERSION << std::endl;
#if defined(WINDOWS)
    std::cout << "Platform: Windows" << std::endl;
#elif defined(LINUX)
    std::cout << "Platform: Linux" << std::endl;
#elif defined(MACOS)
    std::cout << "Platform: MacOS" << std::endl;
#endif

    return 0;
}
