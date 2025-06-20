#include "ServerConfig.h"

namespace pirks::config
{

void ServerConfig::addOptions(CLI::App &args)
{
    args.add_flag("-t,--tcp", isTCP_, "Use TCP/IP for networking");
}

void ServerConfig::parseOptions([[maybe_unused]] CLI::App &args)
{
    if (isTCP_) {
        connectionType_ = TCP;
    }

    if (connectionType_ == Default) {
        connectionType_ = TCP;
    }
}

}; // namespace pirks::config
