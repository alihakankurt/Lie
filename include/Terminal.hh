#pragma once

#include <Core.hh>

#if defined(LIE_UNIX)
#    include <unistd.h>
#    include <termios.h>
#    include <sys/ioctl.h>
#endif

namespace Lie
{
    class Terminal
    {
      public:
        static auto EnableRawMode() -> void;
        static auto DisableRawMode() -> void;

        static auto GetWindowSize() -> Size;

        static auto Read() -> char;
        static auto Write(const void* data, int size) -> bool;

      private:
#if defined(LIE_UNIX)
        static termios original;
#endif
    };
}
