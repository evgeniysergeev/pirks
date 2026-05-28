#include <gtest/gtest.h>

#include "ControlPlaneMessage.h"

TEST(ControlPlaneMessage, ParsesTypeIdAndPayload)
{
    const ControlPlaneMessage message =
            parseControlPlaneText(R"({"type":"ping","id":"42","payload":{"intervalMs":1000}})");

    EXPECT_EQ(message.frameType, ControlPlaneFrameType::Text);
    EXPECT_EQ(message.type, "ping");
    EXPECT_EQ(message.requestId, "42");
    EXPECT_EQ(message.payload.as_object().at("intervalMs").as_int64(), 1000);
}

TEST(ControlPlaneMessage, AcceptsCommandAlias)
{
    const ControlPlaneMessage message = parseControlPlaneText(R"({"command":"hello"})");

    EXPECT_EQ(message.frameType, ControlPlaneFrameType::Text);
    EXPECT_EQ(message.type, "hello");
    EXPECT_EQ(message.requestId, "");
    EXPECT_TRUE(message.payload.is_null());
}

TEST(ControlPlaneMessage, RejectsInvalidJson)
{
    EXPECT_THROW(parseControlPlaneText("{"), ControlPlaneParseError);
}

TEST(ControlPlaneMessage, RejectsNonObjectJson)
{
    EXPECT_THROW(parseControlPlaneText(R"(["ping"])"), ControlPlaneParseError);
}

TEST(ControlPlaneMessage, RejectsMissingType)
{
    EXPECT_THROW(parseControlPlaneText(R"({"payload":{}})"), ControlPlaneParseError);
}

TEST(ControlPlaneMessage, RejectsNonStringId)
{
    EXPECT_THROW(parseControlPlaneText(R"({"type":"ping","id":42})"), ControlPlaneParseError);
}

TEST(ControlPlaneMessage, ParsesBinaryPayload)
{
    std::vector<uint8_t> payload { 1, 2, 3, 4 };

    const ControlPlaneMessage message = parseControlPlaneBinary(std::move(payload));

    EXPECT_EQ(message.frameType, ControlPlaneFrameType::Binary);
    EXPECT_EQ(message.type, "binary");
    EXPECT_EQ(message.binaryPayload, (std::vector<uint8_t> { 1, 2, 3, 4 }));
}
