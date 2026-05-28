#pragma once

#include <boost/json/value.hpp>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

enum class ControlPlaneFrameType
{
    Text,
    Binary,
};

class ControlPlaneParseError final: public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

struct ControlPlaneMessage final
{
    ControlPlaneFrameType frameType { ControlPlaneFrameType::Text };
    std::string           type;
    std::string           requestId;
    boost::json::value    payload;
    std::vector<uint8_t>  binaryPayload;
};

auto parseControlPlaneText(std::string_view text) -> ControlPlaneMessage;
auto parseControlPlaneBinary(std::vector<uint8_t> payload) -> ControlPlaneMessage;
