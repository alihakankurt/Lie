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

        int _cursorX;
        int _cursorY;

        int _scrollX;
        int _scrollY;

        std::vector<Buffer> _lines;

        auto CurrentLine() -> Buffer&;

        auto Initialize() -> void;
        auto Finalize() -> void;

        auto ProcessInput(Key key) -> void;
        auto MoveCursor(Key key) -> void;

        auto RefreshScreen() -> void;
    };
}
