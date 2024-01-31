#include <Editor.hh>

namespace Lie
{
    auto Editor::Start() -> void
    {
        Initialize();

        while (_running)
        {
            RefreshScreen();
            Key key = Terminal::Read();
            ProcessInput(key);
        }

        Finalize();
    }

    auto Editor::CurrentLine() -> Buffer&
    {
        return _lines[_scrollY + _cursorY - 1];
    }

    auto Editor::Initialize() -> void
    {
        _running = true;

        _cursorX = 1;
        _cursorY = 1;

        _scrollX = 0;
        _scrollY = 0;

        Terminal::EnableRawMode();

        Terminal::ClearScreen();
        Terminal::MoveCursor(1, 1);
        Terminal::Flush();

        Size windowSize = Terminal::GetWindowSize();
        _lines.resize(windowSize.Height - 1);
    }

    auto Editor::Finalize() -> void
    {
        Terminal::ClearScreen();
        Terminal::MoveCursor(1, 1);
        Terminal::Flush();
        Terminal::DisableRawMode();
    }

    auto Editor::ProcessInput(Key key) -> void
    {
        switch (key)
        {
            case Key::CtrlQ:
                _running = false;
                break;

            case Key::Up:
            case Key::Down:
            case Key::Left:
            case Key::Right:
                MoveCursor(key);
                break;

            case Key::Backspace:
                if (_cursorX > 1)
                {
                    CurrentLine().Remove(_cursorX - 2);
                    _cursorX--;
                }
                break;

            default:
                if ((key >= Key::A && key <= Key::Z) || (key >= Key::LowerA && key <= Key::LowerZ) || (key >= Key::Zero && key <= Key::Nine) ||
                    key == Key::Space)
                {
                    char c = static_cast<char>(key);
                    CurrentLine().Insert(_cursorX - 1, c);
                    _cursorX++;
                }
                break;
        }
    }

    auto Editor::MoveCursor(Key key) -> void
    {
        Size windowSize = Terminal::GetWindowSize();
        switch (key)
        {
            case Key::Up:
                if (_cursorY > 1)
                    _cursorY--;
                else if (_scrollY > 0)
                    _scrollY--;
                _cursorX = std::min(_cursorX, CurrentLine().Size() - _scrollX + 1);
                break;

            case Key::Down:
                if (_cursorY < windowSize.Height - 1)
                    _cursorY++;
                else if (_scrollY < static_cast<int>(_lines.size()) - windowSize.Height + 1)
                    _scrollY++;
                _cursorX = std::min(_cursorX, CurrentLine().Size() - _scrollX + 1);
                break;

            case Key::Left:
                if (_cursorX > 1)
                    _cursorX = std::min(_cursorX - 1, CurrentLine().Size() - _scrollX + 1);
                else if (_scrollX > 0)
                    _scrollX--;
                break;

            case Key::Right:
                if (_cursorX < CurrentLine().Size() + 1 - _scrollX)
                    _cursorX = std::min(_cursorX + 1, CurrentLine().Size() - _scrollX + 1);
                else if (_scrollX < CurrentLine().Size() - windowSize.Width)
                    _scrollX++;
                break;

            default:
                break;
        }
    }

    // TODO: This function can be abstracted.
    // FIXME: The screen should move completely when scrolling.
    auto Editor::RefreshScreen() -> void
    {
        Terminal::HideCursor();

        Terminal::MoveCursor(1, _cursorY);

        Terminal::Write(CurrentLine().Data() + _scrollX, CurrentLine().Size() - _scrollX);

        Terminal::ClearLine();

        Terminal::MoveCursor(_cursorX, _cursorY);
        Terminal::ShowCursor();

        Terminal::Flush();
    }
}
