#include "ControlPlaneMessage.h"

#include <boost/json/src.hpp>
#include <utility>

namespace
{

auto requiredString(
        const boost::json::object &object,
        std::string_view           primaryKey,
        std::string_view           fallbackKey) -> std::string
{
    const boost::json::value *value = object.if_contains(primaryKey);
    if (value == nullptr) {
        value = object.if_contains(fallbackKey);
    }

    if (value == nullptr) {
        throw ControlPlaneParseError("Control plane message requires a string type or command");
    }

    if (!value->is_string()) {
        throw ControlPlaneParseError("Control plane message type must be a string");
    }

    return std::string(value->as_string());
}

auto optionalString(const boost::json::object &object, std::string_view key) -> std::string
{
    const boost::json::value *value = object.if_contains(key);
    if (value == nullptr) {
        return {};
    }

    if (!value->is_string()) {
        throw ControlPlaneParseError("Control plane message id must be a string");
    }

    return std::string(value->as_string());
}

auto optionalPayload(const boost::json::object &object) -> boost::json::value
{
    const boost::json::value *value = object.if_contains("payload");
    if (value == nullptr) {
        return nullptr;
    }

    return *value;
}

} // namespace

auto parseControlPlaneText(std::string_view text) -> ControlPlaneMessage
{
    boost::system::error_code error;
    boost::json::value        value = boost::json::parse(text, error);
    if (error) {
        throw ControlPlaneParseError("Control plane message is not valid JSON: " + error.message());
    }

    if (!value.is_object()) {
        throw ControlPlaneParseError("Control plane message must be a JSON object");
    }

    const boost::json::object &object = value.as_object();

    ControlPlaneMessage message;
    message.frameType = ControlPlaneFrameType::Text;
    message.type      = requiredString(object, "type", "command");
    message.requestId = optionalString(object, "id");
    message.payload   = optionalPayload(object);

    return message;
}

auto parseControlPlaneBinary(std::vector<uint8_t> payload) -> ControlPlaneMessage
{
    ControlPlaneMessage message;
    message.frameType     = ControlPlaneFrameType::Binary;
    message.type          = "binary";
    message.binaryPayload = std::move(payload);

    return message;
}
