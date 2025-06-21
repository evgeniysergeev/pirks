#pragma once

#include <inttypes.h>

namespace pirks::networking
{

#pragma pack(push, 1)

/**
 * @brief This header used to store packet from network in memory
 *
 */
struct PacketHeader
{
    uint8_t  channel { 0 };
    bool     reliable { false };
    uint16_t reserved { 0 };
    uint32_t size { 0 };
};

/**
 * @brief This structure is used to store inforamtion about packets in memory
 *
 */
struct PacketInfo : public PacketHeader
{
    uint8_t *data { nullptr };
};

#pragma pack(pop)

}; // namespace pirks::networking
