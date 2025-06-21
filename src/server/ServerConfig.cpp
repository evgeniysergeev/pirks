#include <iostream>

#include "ServerConfig.h"

namespace pirks::config
{

void ServerConfig::addOptions(CLI::App &args)
{
    args.add_flag("-t,--tcp", isTCP_, "Use TCP/IP for networking");
    args.add_flag("-u,--udp", isUDP_, "Use UDP for networking");
}

bool ServerConfig::parseOptions([[maybe_unused]] CLI::App &args)
{
    if (isTCP_ && isUDP_) {
        std::cout << "Unable to use both TCP and UDP connection type" << std::endl;
        return false;
    }

    if (isTCP_) {
        connectionType_ = TCP;
    }

    if (isUDP_) {
        connectionType_ = UDP;
    }

    if (connectionType_ == Default) {
        connectionType_ = UDP;
    }

    return true;
}

}; // namespace pirks::config
