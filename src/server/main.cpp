#include <iostream>

#include "str_utils.h"
#include "version.h"

int main() {
    std::cout << PROJECT_NAME << " v"sv << PROJECT_FULL_VERSION << std::endl;

    return 0;
}
