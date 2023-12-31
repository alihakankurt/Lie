#pragma once

#include <Core.hh>
#include <Buffer.hh>
#include <Terminal.hh>

namespace Lie
{
    class Editor
    {
      public:
        auto Start() -> void;

      private:
        Buffer _buffer;

        auto Flush() -> void;
        auto ClearScreen() -> void;
        auto DrawRows() -> void;
    };
}
