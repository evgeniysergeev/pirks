#pragma once

namespace capture
{

enum class CaptureResult : int
{
    OK = 0,
    Reinit,
    Timeout,
    Interrupted,
    Error
};

}; // namespace capture
