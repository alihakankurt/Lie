#include <Editor.hh>

auto Lie::Editor::Start() -> void
{
    _running = true;
    _cursorX = 1;
    _cursorY = 1;

    Terminal::EnableRawMode();

    InitializeScreen();

    while (_running)
    {
        RefreshScreen();
        Key key = Terminal::Read();
        ProcessInput(key);
    }

    Terminal::DisableRawMode();
    Terminal::Write("\033[2J\033[H", 7);
}

auto Lie::Editor::CurrentLine() -> Buffer&
{
    return _lines[_cursorY - 1];
}

auto Lie::Editor::ProcessInput(Key key) -> void
{
    switch (key)
    {
        case Key::CtrlQ:
            _running = false;
            break;

        case Key::Up:
            _cursorY = std::max(_cursorY - 1, 1);
            _cursorX = std::min(_cursorX, CurrentLine().Size() + 1);
            break;

        case Key::Down:
            _cursorY = std::min(_cursorY + 1, Terminal::GetWindowSize().Height - 1);
            _cursorX = std::min(_cursorX, CurrentLine().Size() + 1);
            break;

        case Key::Left:
            _cursorX = std::max(_cursorX - 1, 1);
            break;

        case Key::Right:
            _cursorX = std::min(_cursorX + 1, CurrentLine().Size() + 1);
            break;

        case Key::Backspace:
            if (_cursorX > 1)
            {
                CurrentLine().Remove(_cursorX - 2);
                _cursorX--;
            }
            break;

        default:
            if ((key >= Key::A && key <= Key::Z)
                || (key >= Key::LowerA && key <= Key::LowerZ)
                || (key >= Key::Zero && key <= Key::Nine)
                || key == Key::Space)
            {
                char c = static_cast<char>(key);
                CurrentLine().Insert(_cursorX - 1, c);
                _cursorX++;
            }
            break;
    }
}

auto Lie::Editor::InitializeScreen() -> void
{
    Size windowSize = Terminal::GetWindowSize();
    _lines.resize(windowSize.Height - 1);

    Terminal::Write("\033[2J\033[H", 7);
    for (int y = 0; y < windowSize.Height - 1; y++)
    {
        Terminal::Write(_lines[y].Data(), _lines[y].Size());
    }

    Terminal::Write("\033[H", 3);
}

auto Lie::Editor::RefreshScreen() -> void
{
    _buffer.Clear();

    std::string stringX = std::to_string(_cursorX);
    std::string stringY = std::to_string(_cursorY);

    _buffer.Append("\033[?25l");

    _buffer.Append("\033[");
    _buffer.Append(stringY);
    _buffer.Append(";1H");

    _buffer.Append(CurrentLine());
    _buffer.Append("\033[0K\r\n");

    _buffer.Append("\033[");
    _buffer.Append(stringY);
    _buffer.Append(";");
    _buffer.Append(stringX);
    _buffer.Append("H");

    _buffer.Append("\033[?25h");

    Terminal::Write(_buffer.Data(), _buffer.Size());
}
