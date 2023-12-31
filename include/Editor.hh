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
        bool _running;

        Buffer _buffer;

        int _cursorX;
        int _cursorY;

        auto ProcessInput(Key key) -> void;

        auto Flush() -> void;
        auto RefreshScreen() -> void;
    };
}
