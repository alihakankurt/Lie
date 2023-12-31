#include <Editor.hh>

auto Lie::Editor::Start() -> void
{
    Terminal::EnableRawMode();

    while (1)
    {
        char c = Terminal::Read();
        if (c == 'q')
            break;

        ClearScreen();

        DrawRows();

        Flush();
    }

    Terminal::DisableRawMode();
    Terminal::Write("\x1b[2J\x1b[H", 7);
}

auto Lie::Editor::Flush() -> void
{
    Terminal::Write(_buffer.Data(), _buffer.Size());
    _buffer.Clear();
}

auto Lie::Editor::ClearScreen() -> void
{
    _buffer.Append("\x1b[2J");
    _buffer.Append("\x1b[H");
}

auto Lie::Editor::DrawRows() -> void
{
    Size size = Terminal::GetWindowSize();

    for (int y = 0; y < size.Height; y++)
    {
        _buffer.Append("~");
        if (y < size.Height - 1)
            _buffer.Append("\r\n");
    }

    _buffer.Append("\x1b[H");
}
