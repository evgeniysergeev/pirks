#pragma once

namespace audio
{

enum class CaptureResult : int
{
    OK = 0,      ///< Success
    Reinit,      ///< Need to reinitialize
    Timeout,     ///< Timeout
    Interrupted, ///< Capture was interrupted
    Error        ///< Error
};

}; // namespace audio
