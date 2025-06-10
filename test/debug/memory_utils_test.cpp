#include "common/debug/memory_utils.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <array>
#include <sstream>
#include <string>
#include <vector>

class MemoryUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Настраиваем spdlog для тестов
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("[%l] %v");
    }
};

TEST_F(MemoryUtilsTest, DumpMemoryToString_ConstBasic)
{
    const std::vector<uint8_t> data = { 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57,
                                        0x6f, 0x72, 0x6c, 0x64, 0x21, 0x00, 0x00, 0x00 };

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    std::string expected =
            "00000000: 48 65 6c 6c 6f 2c 20 57  6f 72 6c 64 21 00 00 00 |Hello, World!...|\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_NonConstBasic)
{
    std::vector<uint8_t> data = { 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57,
                                  0x6f, 0x72, 0x6c, 0x64, 0x21, 0x00, 0x00, 0x00 };

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    std::string expected =
            "00000000: 48 65 6c 6c 6f 2c 20 57  6f 72 6c 64 21 00 00 00 |Hello, World!...|\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_WithPrefix)
{
    std::vector<uint8_t> data   = { 0x48, 0x65, 0x6c, 0x6c, 0x6f };
    std::string          result = memory_utils::dump_memory_to_string(std::span { data }, "TEST: ");

    std::string expected =
            "TEST: 00000000: 48 65 6c 6c 6f 00 00 00  00 00 00 00 00 00 00 00 |Hello...........|\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_NonPrintable)
{
    std::vector<uint8_t> data = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    std::string expected =
            "00000000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f |................|\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_ConstTemplate)
{
    const std::string data   = "Hello";
    std::string       result = memory_utils::dump_memory_to_string(std::span { data });

    // Проверяем, что строка содержит правильные байты
    EXPECT_TRUE(result.find("48 65 6c 6c 6f") != std::string::npos);
    EXPECT_TRUE(result.find("Hello") != std::string::npos);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_NonConstTemplate)
{
    std::string data   = "Hello";
    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Проверяем, что строка содержит правильные байты
    EXPECT_TRUE(result.find("48 65 6c 6c 6f") != std::string::npos);
    EXPECT_TRUE(result.find("Hello") != std::string::npos);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_CustomStruct)
{
    struct TestStruct
    {
        int32_t x;
        char    c;
        double  d;
    };

    std::vector<TestStruct> data = {
        { 1, 'a',  3.14 },
        { 2, 'b', 2.718 }
    };
    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Проверяем, что вывод содержит данные структуры
    EXPECT_TRUE(result.find("01 00 00 00") != std::string::npos); // int32_t 1
    EXPECT_TRUE(result.find("61") != std::string::npos);          // char 'a'
    EXPECT_TRUE(result.find("02 00 00 00") != std::string::npos); // int32_t 2
    EXPECT_TRUE(result.find("62") != std::string::npos);          // char 'b'
}

TEST_F(MemoryUtilsTest, LogMemoryDump_Levels)
{
    std::vector<uint8_t> data = { 0x48, 0x65, 0x6c, 0x6c, 0x6f };

    // Тестируем разные уровни логирования
    testing::internal::CaptureStdout();
    memory_utils::log_memory_dump(std::span { data }, "", spdlog::level::debug);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("[debug]") != std::string::npos);
    EXPECT_TRUE(output.find("48 65 6c 6c 6f") != std::string::npos);
}

TEST_F(MemoryUtilsTest, LogMemoryDump_EmptyData)
{
    std::vector<uint8_t> data;

    testing::internal::CaptureStdout();
    memory_utils::log_memory_dump(std::span { data });
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.empty());
}

TEST_F(MemoryUtilsTest, LogMemoryDump_LargeData)
{
    // Создаем данные размером больше 16 байт
    std::vector<uint8_t> data(32);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i);
    }

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Проверяем, что вывод содержит две строки
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 2);

    // Проверяем, что вторая строка начинается с правильного смещения
    EXPECT_TRUE(result.find("00000010:") != std::string::npos);
}

TEST_F(MemoryUtilsTest, LogMemoryDump_Array)
{
    std::array<uint8_t, 5> data = { 0x48, 0x65, 0x6c, 0x6c, 0x6f };

    testing::internal::CaptureStdout();
    memory_utils::log_memory_dump(std::span { data });
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("48 65 6c 6c 6f") != std::string::npos);
    EXPECT_TRUE(output.find("Hello") != std::string::npos);
}

TEST_F(MemoryUtilsTest, LogMemoryDump_ModifyData)
{
    std::vector<uint8_t> data = { 0x48, 0x65, 0x6c, 0x6c, 0x6f };

    // Модифицируем данные после создания span
    auto span = std::span { data };
    data[0]   = 0x57; // 'W' вместо 'H'

    std::string result = memory_utils::dump_memory_to_string(span);
    EXPECT_TRUE(result.find("57 65 6c 6c 6f") != std::string::npos);
    EXPECT_TRUE(result.find("Wello") != std::string::npos);
}
