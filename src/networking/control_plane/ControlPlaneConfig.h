#pragma once

#include <cstdint>
#include <string>

struct ControlPlaneConfig final
{
    std::string bindAddress { "0.0.0.0" };
    uint16_t    port { 5102 };
    std::string certificateChainFile;
    std::string privateKeyFile;
};
