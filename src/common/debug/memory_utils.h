#pragma once

#include <spdlog/spdlog.h>

#include <cstdint>
#include <span>
#include <string>

/**
 * @namespace memory_utils
 * @brief Namespace containing utilities for memory inspection and debugging
 */
namespace memory_utils
{

/**
 * @brief Converts memory contents to a string in hexadecimal and ASCII format
 *
 * This function formats the contents of a memory region in a format similar to hex editors.
 * Each line shows:
 * - Memory offset in hexadecimal
 * - 16 bytes of data in hexadecimal format, split into two 8-byte groups
 * - ASCII representation of the same data (non-printable characters shown as dots)
 *
 * Example output:
 * @code
 * 00000000: 48 65 6c 6c 6f 2c 20 57  6f 72 6c 64 21 00 00 00  |Hello, W orld!...|
 * @endcode
 *
 * @param data A span containing the memory region to dump
 * @param prefix Optional string to prepend to each line of output
 * @return std::string containing the formatted memory dump
 *
 * @see dumpMemoryToString(const std::span<const T>&, const std::string&)
 */
std::string dumpMemoryToString(std::span<const std::uint8_t> data, const std::string &prefix = "");

/**
 * @brief Overload for non-const byte spans
 * @see dumpMemoryToString(const std::span<const std::uint8_t>&, const std::string&)
 */
std::string dumpMemoryToString(std::span<std::uint8_t> data, const std::string &prefix = "");

/**
 * @brief Template overload for converting memory of any type to string
 *
 * This template function allows dumping memory of any type by converting it to a byte span.
 * It's particularly useful for inspecting the raw memory representation of objects.
 *
 * @tparam T The type of data to dump (must be a trivially copyable type)
 * @param data A span containing the data to dump
 * @param prefix Optional string to prepend to each line of output
 * @return std::string containing the formatted memory dump
 *
 * @note This function performs a reinterpret_cast of the data to bytes, so it should
 *       only be used with trivially copyable types to avoid undefined behavior
 *
 * @see dumpMemoryToString(const std::span<const std::uint8_t>&, const std::string&)
 */
template<typename T>
std::string dumpMemoryToString(std::span<const T> data, const std::string &prefix = "")
{
    return dumpMemoryToString(
            std::span<const std::uint8_t>(
                    reinterpret_cast<const std::uint8_t *>(data.data()),
                    data.size() * sizeof(T)),
            prefix);
}

/**
 * @brief Template overload for non-const types
 * @see dumpMemoryToString(const std::span<const T>&, const std::string&)
 */
template<typename T>
std::string dumpMemoryToString(std::span<T> data, const std::string &prefix = "")
{
    return dumpMemoryToString(
            std::span<std::uint8_t>(
                    reinterpret_cast<std::uint8_t *>(data.data()),
                    data.size() * sizeof(T)),
            prefix);
}

/**
 * @brief Logs memory contents using spdlog
 *
 * This function formats the memory contents and logs them using spdlog.
 * The output format is the same as in dumpMemoryToString().
 *
 * @param data A span containing the memory region to dump
 * @param prefix Optional string to prepend to each line of output
 * @param level The spdlog log level to use (default: info)
 *
 * @see dumpMemoryToString()
 */
void logMemoryDump(
        std::span<const std::uint8_t> data,
        const std::string            &prefix = "",
        spdlog::level::level_enum     level  = spdlog::level::info);

/**
 * @brief Overload for non-const byte spans
 * @see logMemoryDump(const std::span<const std::uint8_t>&, const std::string&,
 * spdlog::level::level_enum)
 */
void logMemoryDump(
        std::span<std::uint8_t>   data,
        const std::string        &prefix = "",
        spdlog::level::level_enum level  = spdlog::level::info);

/**
 * @brief Template overload for logging memory of any type
 *
 * @tparam T The type of data to dump (must be a trivially copyable type)
 * @param data A span containing the data to dump
 * @param prefix Optional string to prepend to each line of output
 * @param level The spdlog log level to use (default: info)
 *
 * @see logMemoryDump(const std::span<const std::uint8_t>&, const std::string&,
 * spdlog::level::level_enum)
 */
template<typename T>
void logMemoryDump(
        std::span<const T>        data,
        const std::string        &prefix = "",
        spdlog::level::level_enum level  = spdlog::level::info)
{
    logMemoryDump(
            std::span<const std::uint8_t>(
                    reinterpret_cast<const std::uint8_t *>(data.data()),
                    data.size() * sizeof(T)),
            prefix,
            level);
}

/**
 * @brief Template overload for non-const types
 * @see logMemoryDump(const std::span<const T>&, const std::string&, spdlog::level::level_enum)
 */
template<typename T>
void logMemoryDump(
        std::span<T>              data,
        const std::string        &prefix = "",
        spdlog::level::level_enum level  = spdlog::level::info)
{
    logMemoryDump(
            std::span<std::uint8_t>(
                    reinterpret_cast<std::uint8_t *>(data.data()),
                    data.size() * sizeof(T)),
            prefix,
            level);
}

} // namespace memory_utils
