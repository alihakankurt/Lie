#include <Editor.hh>

auto Lie::Editor::Start() -> void
{
    _running = true;
    _cursorX = 1;
    _cursorY = 1;

    Terminal::EnableRawMode();

    while (_running)
    {
        RefreshScreen();
        Key key = Terminal::Read();
        ProcessInput(key);
    }

    Terminal::DisableRawMode();
    Terminal::Write("\033[2J\033[H", 7);
}

auto Lie::Editor::ProcessInput(Key key) -> void
{
    switch (key)
    {
        case Key::Q:
        case Key::LowerQ:
            _running = false;
            break;

        case Key::Up:
            _cursorY = std::max(_cursorY - 1, 1);
            break;

        case Key::Down:
            _cursorY = std::min(_cursorY + 1, Terminal::GetWindowSize().Height - 1);
            break;

        case Key::Left:
            _cursorX = std::max(_cursorX - 1, 1);
            break;

        case Key::Right:
            _cursorX = std::min(_cursorX + 1, Terminal::GetWindowSize().Width);
            break;

        default:
            break;
    }
}

auto Lie::Editor::Flush() -> void
{
    Terminal::Write(_buffer.Data(), _buffer.Size());
    _buffer.Clear();
}

auto Lie::Editor::RefreshScreen() -> void
{
    // Hide cursor
    _buffer.Append("\033[?25l");

    // Clear screen
    _buffer.Append("\033[H");

    // Draw rows
    Size size = Terminal::GetWindowSize();

    for (int y = 1; y < size.Height; y++)
    {
        _buffer.Append("~\033[0K\r\n");
    }

    std::string stringX = std::to_string(_cursorX);
    std::string stringY = std::to_string(_cursorY);

    // Draw status bar
    _buffer.Append("\033[7m");
    _buffer.Append("Lie - ");
    _buffer.Append(stringY);
    _buffer.Append(":");
    _buffer.Append(stringX);
    _buffer.Append("\033[m");
    _buffer.Append("\033[0K\r\n");

    // Move cursor
    _buffer.Append("\033[");
    _buffer.Append(stringY);
    _buffer.Append(";");
    _buffer.Append(stringX);
    _buffer.Append("H");

    // Show cursor
    _buffer.Append("\033[?25h");

    Flush();
}
