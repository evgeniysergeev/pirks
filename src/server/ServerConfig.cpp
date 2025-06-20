#include "ServerConfig.h"

namespace pirks::config
{

void ServerConfig::addOptions(CLI::App &args)
{
    args.add_flag("-t,--tcp", isTCP_, "Use TCP/IP for networking (not implemented yet)");
    args.add_flag("-u,--udp", isUDP_, "Use UDP for networking");
}

void ServerConfig::parseOptions([[maybe_unused]] CLI::App &args)
{
    if (isTCP_) {
        connectionType_ = TCP;
    }

    if (isUDP_) {
        connectionType_ = UDP;
    }

    if (connectionType_ == Default) {
        connectionType_ = UDP;
    }
}

}; // namespace pirks::config
