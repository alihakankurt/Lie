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
        std::vector<Buffer> _lines;

        int _cursorX;
        int _cursorY;

        auto CurrentLine() -> Buffer&;

        auto ProcessInput(Key key) -> void;

        auto InitializeScreen() -> void;
        auto RefreshScreen() -> void;
    };
}
