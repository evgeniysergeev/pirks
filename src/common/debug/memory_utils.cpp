#include "memory_utils.h"

#include <spdlog/spdlog.h>

#include <iomanip>
#include <sstream>

namespace memory_utils
{

std::string dump_memory_to_string(std::span<const std::uint8_t> data, const std::string &prefix)
{
    std::stringstream ss;

    for (size_t i = 0; i < data.size(); i += 16) {
        ss << prefix;

        // Output memory offset
        ss << std::hex << std::setw(8) << std::setfill('0') << i << ": ";

        // Output hexadecimal values
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < data.size()) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i + j])
                   << " ";
            } else {
                ss << "   ";
            }

            // Add space after 8 bytes
            if (j == 7) {
                ss << " ";
            }
        }

        // Output ASCII representation
        ss << " |";
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < data.size()) {
                char c = static_cast<char>(data[i + j]);
                // Replace non-printable characters with dots
                ss << (c >= 32 && c <= 126 ? c : '.');
            } else {
                ss << " ";
            }

            // Add space after 8 bytes
            if (j == 7) {
                ss << " ";
            }
        }
        ss << "|\n";
    }

    return ss.str();
}

std::string dump_memory_to_string(std::span<std::uint8_t> data, const std::string &prefix)
{
    return dump_memory_to_string(std::span<const std::uint8_t>(data), prefix);
}

void log_memory_dump(
        std::span<const std::uint8_t> data,
        const std::string            &prefix,
        spdlog::level::level_enum     level)
{
    std::string dump = dump_memory_to_string(data, prefix);

    // Split the dump into lines and log each line separately
    std::istringstream iss(dump);
    std::string        line;
    while (std::getline(iss, line)) {
        switch (level) {
        case spdlog::level::trace:
            spdlog::trace("{}", line);
            break;
        case spdlog::level::debug:
            spdlog::debug("{}", line);
            break;
        case spdlog::level::info:
            spdlog::info("{}", line);
            break;
        case spdlog::level::warn:
            spdlog::warn("{}", line);
            break;
        case spdlog::level::err:
            spdlog::error("{}", line);
            break;
        case spdlog::level::critical:
            spdlog::critical("{}", line);
            break;
        default:
            spdlog::info("{}", line);
            break;
        }
    }
}

void log_memory_dump(
        std::span<std::uint8_t>   data,
        const std::string        &prefix,
        spdlog::level::level_enum level)
{
    log_memory_dump(std::span<const std::uint8_t>(data), prefix, level);
}

} // namespace memory_utils
