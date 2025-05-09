#pragma once

namespace capture
{

enum class CaptureResult : int
{
    OK = 0,      ///< Success
    Reinit,      ///< Need to reinitialize
    Timeout,     ///< Timeout
    Interrupted, ///< Capture was interrupted
    Error        ///< Error
};

}; // namespace capture
