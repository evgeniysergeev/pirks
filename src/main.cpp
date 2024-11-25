#include <iostream>

#include "version.h"
#include "common/str_utils.h"

int main()
{
    std::cout
        << PROJECT_NAME
        << " v"sv << PROJECT_VERSION
#ifdef DEBUG
        << " (Debug build)"
#endif
        << std::endl;

    return 0;
}
