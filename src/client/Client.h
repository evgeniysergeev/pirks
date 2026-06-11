#pragma once

#include <memory>

#include "ClientConfig.h"

class ControlPlaneClient;

class Client final
{
public:
    explicit Client(ClientConfig config);
    ~Client();

public:
    void run();

private:
    ClientConfig                        config_;
    std::unique_ptr<ControlPlaneClient> controlPlaneClient_;
};
