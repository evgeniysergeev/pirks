#include "debug/memory_utils.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <vector>

class MemoryUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Configure spdlog for tests
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("[%l] %v");
    }
};

TEST_F(MemoryUtilsTest, DumpMemoryToString_Basic)
{
    std::vector<uint8_t> data = { 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57,
                                  0x6f, 0x72, 0x6c, 0x64, 0x21, 0x00, 0x00, 0x00 };

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    std::string expected =
            "00000000: 48 65 6c 6c 6f 2c 20 57  6f 72 6c 64 21 00 00 00  |Hello, W orld!...|\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_WithPrefix)
{
    std::vector<uint8_t> data   = { 0x48, 0x65, 0x6c, 0x6c, 0x6f };
    std::string          result = memory_utils::dump_memory_to_string(std::span { data }, "TEST: ");

    std::string expected =
            "TEST: 00000000: 48 65 6c 6c 6f                                    |Hello            |\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_NonPrintable)
{
    std::vector<uint8_t> data = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    std::string expected =
            "00000000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  |........ ........|\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_TemplateOverload)
{
    std::string data   = "Hello";
    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Проверяем, что строка содержит правильные байты
    EXPECT_TRUE(result.find("48 65 6c 6c 6f") != std::string::npos);
    EXPECT_TRUE(result.find("Hello") != std::string::npos);
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

TEST_F(MemoryUtilsTest, DumpMemoryToString_MultiLine)
{
    // Создаем данные размером 48 байт (3 строки по 16 байт)
    std::vector<uint8_t> data(48);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i);
    }

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Проверяем наличие всех трех строк
    EXPECT_TRUE(result.find("00000000:") != std::string::npos);
    EXPECT_TRUE(result.find("00000010:") != std::string::npos);
    EXPECT_TRUE(result.find("00000020:") != std::string::npos);

    // Проверяем количество строк
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 3);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_ExactLineSize)
{
    // Создаем данные размером ровно 16 байт
    std::vector<uint8_t> data(16);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i);
    }

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Проверяем, что вывод содержит только одну строку
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 1);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_EmptyData)
{
    std::vector<uint8_t> data;
    std::string          result = memory_utils::dump_memory_to_string(std::span { data });
    EXPECT_TRUE(result.empty());
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_SingleByte)
{
    std::vector<uint8_t> data   = { 0x42 };
    std::string          result = memory_utils::dump_memory_to_string(std::span { data });

    std::string expected =
            "00000000: 42                                                |B                |\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_AllPrintable)
{
    std::vector<uint8_t> data;
    for (uint8_t i = 32; i <= 126; ++i) { // ASCII printable characters
        data.push_back(i);
    }

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Check that all characters are displayed correctly
    EXPECT_TRUE(result.find(" !\"#$%&' ()*+,-./") != std::string::npos);
    EXPECT_TRUE(result.find("01234567 89:;<=>?") != std::string::npos);
    EXPECT_TRUE(result.find("@ABCDEFG HIJKLMNO") != std::string::npos);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_AllNonPrintable)
{
    std::vector<uint8_t> data;
    for (uint8_t i = 0; i < 32; ++i) { // ASCII control characters
        data.push_back(i);
    }

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Check that all non-printable characters are displayed as dots
    EXPECT_TRUE(result.find("........ ........") != std::string::npos);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_Unicode)
{
    // Test Unicode characters (UTF-8)
    std::string data   = "Привет, мир!";
    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Check that Unicode characters are correctly displayed in hex
    EXPECT_TRUE(result.find("d0 9f") != std::string::npos); // 'П'
    EXPECT_TRUE(result.find("d1 80") != std::string::npos); // 'р'
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_LargeOffset)
{
    // Создаем данные с большим смещением
    std::vector<uint8_t> data(32);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i);
    }

    // Создаем span с большим смещением
    auto        span   = std::span { data }.subspan(16);
    std::string result = memory_utils::dump_memory_to_string(span);

    // Проверяем, что смещение корректно отображается
    EXPECT_TRUE(result.find("00000000:") != std::string::npos);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_ZeroBytes)
{
    std::vector<uint8_t> data(16, 0);
    std::string          result = memory_utils::dump_memory_to_string(std::span { data });

    std::string expected =
            "00000000: 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |........ ........|\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_AlternatingBytes)
{
    std::vector<uint8_t> data   = { 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
                                    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF };
    std::string          result = memory_utils::dump_memory_to_string(std::span { data });

    std::string expected =
            "00000000: 00 ff 00 ff 00 ff 00 ff  00 ff 00 ff 00 ff 00 ff  |........ ........|\n";
    EXPECT_EQ(result, expected);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_LongPrefix)
{
    std::vector<uint8_t> data   = { 0x48, 0x65, 0x6c, 0x6c, 0x6f };
    std::string          prefix = "VERY_LONG_PREFIX_THAT_SHOULD_NOT_BREAK_FORMATTING: ";
    std::string          result = memory_utils::dump_memory_to_string(std::span { data }, prefix);

    // Проверяем, что длинный префикс корректно обрабатывается
    EXPECT_TRUE(result.find(prefix) != std::string::npos);
    EXPECT_TRUE(result.find("48 65 6c 6c 6f") != std::string::npos);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_ThreeLines_Detailed)
{
    // Создаем данные размером 48 байт (3 строки по 16 байт)
    std::vector<uint8_t> data(48);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i);
    }

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Проверяем первую строку
    std::string expected =
            "00000000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  |........ ........|\n"
            "00000010: 10 11 12 13 14 15 16 17  18 19 1a 1b 1c 1d 1e 1f  |........ ........|\n"
            "00000020: 20 21 22 23 24 25 26 27  28 29 2a 2b 2c 2d 2e 2f  | !\"#$%&' ()*+,-./|\n";

    EXPECT_EQ(result, expected);

    // Проверяем общее количество строк
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 3);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_FiveLines_Detailed)
{
    // Создаем данные размером 80 байт (5 строк по 16 байт)
    std::vector<uint8_t> data(80);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i);
    }

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Проверяем все пять строк
    std::string expected =
            "00000000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  |........ ........|\n"
            "00000010: 10 11 12 13 14 15 16 17  18 19 1a 1b 1c 1d 1e 1f  |........ ........|\n"
            "00000020: 20 21 22 23 24 25 26 27  28 29 2a 2b 2c 2d 2e 2f  | !\"#$%&' ()*+,-./|\n"
            "00000030: 30 31 32 33 34 35 36 37  38 39 3a 3b 3c 3d 3e 3f  |01234567 89:;<=>?|\n"
            "00000040: 40 41 42 43 44 45 46 47  48 49 4a 4b 4c 4d 4e 4f  |@ABCDEFG HIJKLMNO|\n";

    EXPECT_EQ(result, expected);

    // Проверяем общее количество строк
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 5);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_ThreeLines_WithPrintable)
{
    // Создаем данные размером 48 байт (3 строки по 16 байт)
    // Первая строка - непечатаемые символы
    // Вторая строка - смесь печатаемых и непечатаемых
    // Третья строка - все печатаемые
    // clang-format off
    std::vector<uint8_t> data = {
        // Первая строка (непечатаемые)
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        // Вторая строка (смесь)
        0x20, 0x21, 0x22, 0x00, 0x01, 0x02, 0x23, 0x24,
        0x25, 0x00, 0x01, 0x26, 0x27, 0x28, 0x29, 0x2A,
        // Третья строка (печатаемые)
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50
    };
    // clang-format on

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Check first line (all dots)
    std::string expected =
            "00000000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  |........ ........|\n"
            // Check second line (mix of dots and characters)
            "00000010: 20 21 22 00 01 02 23 24  25 00 01 26 27 28 29 2a  | !\"...#$ %..&'()*|\n"
            // Check third line (all characters)
            "00000020: 41 42 43 44 45 46 47 48  49 4a 4b 4c 4d 4e 4f 50  |ABCDEFGH IJKLMNOP|\n";

    EXPECT_EQ(result, expected);

    // Check total number of lines
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 3);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_LastLinePartial_Short)
{
    // Create data: 2 full lines (32 bytes) + 5 bytes in the last line
    std::vector<uint8_t> data(38);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i);
    }

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Check first two full lines
    std::string expected =
            "00000000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  |........ ........|\n"
            "00000010: 10 11 12 13 14 15 16 17  18 19 1a 1b 1c 1d 1e 1f  |........ ........|\n"
            // Check last partial line (5 bytes + padding)
            "00000020: 20 21 22 23 24 25                                 | !\"#$%           |\n";

    EXPECT_EQ(result, expected);

    // Check total number of lines
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 3);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_LastLinePartial_Long)
{
    // Create data: 3 full lines (48 bytes) + 12 bytes in the last line
    std::vector<uint8_t> data(60);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i);
    }

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Check first three full lines
    std::string expected =
            "00000000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  |........ ........|\n"
            "00000010: 10 11 12 13 14 15 16 17  18 19 1a 1b 1c 1d 1e 1f  |........ ........|\n"
            "00000020: 20 21 22 23 24 25 26 27  28 29 2a 2b 2c 2d 2e 2f  | !\"#$%&' ()*+,-./|\n"
            // Check last partial line (12 bytes + padding)
            "00000030: 30 31 32 33 34 35 36 37  38 39 3a 3b              |01234567 89:;    |\n";

    EXPECT_EQ(result, expected);

    // Check total number of lines
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 4);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_LastLinePartial_Printable)
{
    // Создаем данные: 2 полные строки (32 байта) + 7 байт в последней строке
    // Используем печатаемые символы для проверки ASCII-представления
    // clang-format off
    std::vector<uint8_t> data = {
        // Первая строка (16 байт)
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
        // Вторая строка (16 байт)
        0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
        // Последняя неполная строка (7 байт)
        0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D
    };
    // clang-format on

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Check first two full lines
    std::string expected =
            "00000000: 41 42 43 44 45 46 47 48  49 4a 4b 4c 4d 4e 4f 50  |ABCDEFGH IJKLMNOP|\n"
            "00000010: 51 52 53 54 55 56 57 58  59 5a 61 62 63 64 65 66  |QRSTUVWX YZabcdef|\n"
            // Check last partial line (7 bytes + padding)
            "00000020: 67 68 69 6a 6b 6c 6d                              |ghijklm          |\n";

    EXPECT_EQ(result, expected);

    // Check total number of lines
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 3);
}

TEST_F(MemoryUtilsTest, DumpMemoryToString_LastLinePartial_Mixed)
{
    // Создаем данные: 2 полные строки (32 байта) + 9 байт в последней строке
    // Смесь печатаемых и непечатаемых символов
    // clang-format off
    std::vector<uint8_t> data = {
        // Первая строка (16 байт) - все печатаемые
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
        // Вторая строка (16 байт) - смесь
        0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44,
        0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48,
        // Последняя неполная строка (9 байт) - смесь
        0x00, 0x49, 0x00, 0x4A, 0x00, 0x4B, 0x00, 0x4C, 0x00
    };
    // clang-format on

    std::string result = memory_utils::dump_memory_to_string(std::span { data });

    // Check first two full lines
    std::string expected =
            "00000000: 41 42 43 44 45 46 47 48  49 4a 4b 4c 4d 4e 4f 50  |ABCDEFGH IJKLMNOP|\n"
            "00000010: 00 41 00 42 00 43 00 44  00 45 00 46 00 47 00 48  |.A.B.C.D .E.F.G.H|\n"
            // Check last partial line (9 bytes + padding)
            "00000020: 00 49 00 4a 00 4b 00 4c  00                       |.I.J.K.L .       |\n";

    EXPECT_EQ(result, expected);

    // Check total number of lines
    size_t newline_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newline_count, 3);
}
