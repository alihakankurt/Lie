#pragma once

#define LIE_VERSION "0.1"

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#    define LIE_UNIX
#elif defined(_WIN32) || defined(_WIN64)
#    error "Windows is not supported yet"
#    define LIE_WINDOWS
#endif

#include <string>
#include <vector>

namespace Lie
{
    struct Size
    {
        int Width;
        int Height;
    };
}
