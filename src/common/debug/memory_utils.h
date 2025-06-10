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
 * 00000000: 48 65 6c 6c 6f 2c 20 57  6f 72 6c 64 21 00 00 00  |Hello, World!...|
 * @endcode
 *
 * @param data A span containing the memory region to dump
 * @param prefix Optional string to prepend to each line of output
 * @return std::string containing the formatted memory dump
 *
 * @see dump_memory_to_string(const std::span<const T>&, const std::string&)
 */
std::string dump_memory_to_string(
        std::span<const std::uint8_t> data,
        const std::string            &prefix = "");

/**
 * @brief Overload for non-const byte spans
 * @see dump_memory_to_string(const std::span<const std::uint8_t>&, const std::string&)
 */
std::string dump_memory_to_string(std::span<std::uint8_t> data, const std::string &prefix = "");

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
 * @see dump_memory_to_string(const std::span<const std::uint8_t>&, const std::string&)
 */
template<typename T>
std::string dump_memory_to_string(std::span<const T> data, const std::string &prefix = "")
{
    return dump_memory_to_string(
            std::span<const std::uint8_t>(
                    reinterpret_cast<const std::uint8_t *>(data.data()),
                    data.size() * sizeof(T)),
            prefix);
}

/**
 * @brief Template overload for non-const types
 * @see dump_memory_to_string(const std::span<const T>&, const std::string&)
 */
template<typename T>
std::string dump_memory_to_string(std::span<T> data, const std::string &prefix = "")
{
    return dump_memory_to_string(
            std::span<std::uint8_t>(
                    reinterpret_cast<std::uint8_t *>(data.data()),
                    data.size() * sizeof(T)),
            prefix);
}

/**
 * @brief Logs memory contents using spdlog
 *
 * This function formats the memory contents and logs them using spdlog.
 * The output format is the same as in dump_memory_to_string().
 *
 * @param data A span containing the memory region to dump
 * @param prefix Optional string to prepend to each line of output
 * @param level The spdlog log level to use (default: info)
 *
 * @see dump_memory_to_string()
 */
void log_memory_dump(
        std::span<const std::uint8_t> data,
        const std::string            &prefix = "",
        spdlog::level::level_enum     level  = spdlog::level::info);

/**
 * @brief Overload for non-const byte spans
 * @see log_memory_dump(const std::span<const std::uint8_t>&, const std::string&,
 * spdlog::level::level_enum)
 */
void log_memory_dump(
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
 * @see log_memory_dump(const std::span<const std::uint8_t>&, const std::string&,
 * spdlog::level::level_enum)
 */
template<typename T>
void log_memory_dump(
        std::span<const T>        data,
        const std::string        &prefix = "",
        spdlog::level::level_enum level  = spdlog::level::info)
{
    log_memory_dump(
            std::span<const std::uint8_t>(
                    reinterpret_cast<const std::uint8_t *>(data.data()),
                    data.size() * sizeof(T)),
            prefix,
            level);
}

/**
 * @brief Template overload for non-const types
 * @see log_memory_dump(const std::span<const T>&, const std::string&, spdlog::level::level_enum)
 */
template<typename T>
void log_memory_dump(
        std::span<T>              data,
        const std::string        &prefix = "",
        spdlog::level::level_enum level  = spdlog::level::info)
{
    log_memory_dump(
            std::span<std::uint8_t>(
                    reinterpret_cast<std::uint8_t *>(data.data()),
                    data.size() * sizeof(T)),
            prefix,
            level);
}

} // namespace memory_utils
