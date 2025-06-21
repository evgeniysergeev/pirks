#pragma once

namespace pirks
{

enum ExitCode : int
{
    OK                 = 0,   ///< Normal program termination
    ConfigurationError = 100, ///< Error in arguments or options
    ExceptionThrown           ///< Exception was thrown
};

}; // namespace pirks
