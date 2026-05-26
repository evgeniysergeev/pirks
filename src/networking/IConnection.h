#pragma once

#include "CircularBuffer.h"
#include "PacketInfo.h"

using PacketsQueue = CircularBuffer<PacketInfo>;

class IConnection
{
public:
    virtual ~IConnection() = default;

public:
    virtual void create(
            std::shared_ptr<PacketsQueue> in_packets,
            std::shared_ptr<PacketsQueue> out_packets) = 0;
};
