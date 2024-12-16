#include <spdlog/spdlog.h>

#include "str_utils.h"
#include "version.h"

int main()
{
    spdlog::info("{} v{} (Platform: {})"sv, PROJECT_NAME, PROJECT_FULL_VERSION, PROJECT_PLATFORM);

    return 0;
}
