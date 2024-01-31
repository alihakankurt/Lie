#include <Terminal.hh>

namespace Lie
{
    auto Terminal::buffer = Buffer();

#if defined(LIE_UNIX)

    auto Terminal::original = termios();

    auto Terminal::EnableRawMode() -> void
    {
        tcgetattr(STDIN_FILENO, &original);
        std::atexit(DisableRawMode);

        termios raw = original;
        raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
        raw.c_oflag &= ~(OPOST);
        raw.c_cflag |= (CS8);
        raw.c_lflag &= ~(ECHO | ISIG | ICANON | IEXTEN);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    auto Terminal::DisableRawMode() -> void
    {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
    }

    auto Terminal::GetWindowSize() -> Size
    {
        winsize ws;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
        return { ws.ws_col, ws.ws_row - 1 };
    }

    auto Terminal::Read() -> Key
    {
        char c, n;

        while ((n = read(STDIN_FILENO, &c, 1)) != 1)
            if (n == -1 && errno != EAGAIN)
                std::exit(1);

        if (c != '\033')
            return (c == 127) ? Key::Backspace : Key(c);

        if (read(STDIN_FILENO, &c, 1) == 0)
            return Key::Escape;

        if (c == '[')
        {
            if (read(STDIN_FILENO, &c, 1) == 0)
                return Key::Escape;

            if (c >= '0' && c <= '9')
            {
                if (read(STDIN_FILENO, &n, 1) == 0 || n != '~')
                    return Key::Escape;

                switch (c)
                {
                    case '1':
                        return Key::Home;
                    case '2':
                        return Key::Insert;
                    case '3':
                        return Key::Delete;
                    case '4':
                        return Key::End;
                    case '5':
                        return Key::PageUp;
                    case '6':
                        return Key::PageDown;
                    case '7':
                        return Key::Home;
                    case '8':
                        return Key::End;
                }
            }
            else
            {
                switch (c)
                {
                    case 'A':
                        return Key::Up;
                    case 'B':
                        return Key::Down;
                    case 'C':
                        return Key::Right;
                    case 'D':
                        return Key::Left;
                    case 'H':
                        return Key::Home;
                    case 'F':
                        return Key::End;
                }
            }
        }
        else if (c == 'O')
        {
            if (read(STDIN_FILENO, &c, 1) == 0)
                return Key::Escape;

            switch (c)
            {
                case 'H':
                    return Key::Home;
                case 'F':
                    return Key::End;
            }
        }

        return Key::Escape;
    }

    auto Terminal::Write(const char* data, int size) -> void
    {
        buffer.Append(data, size);
    }

    auto Terminal::Flush() -> bool
    {
        bool result = write(STDOUT_FILENO, buffer.Data(), buffer.Size()) == buffer.Size();
        buffer.Clear();
        return result;
    }

    auto Terminal::ClearScreen() -> void
    {
        Write("\033[2J\033[1;1H", 10);
    }

    auto Terminal::ClearLine() -> void
    {
        Write("\033[K", 3);
    }

    auto Terminal::MoveCursor(int x, int y) -> void
    {
        static char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "\033[%d;%dH", y, x);
        Write(buffer, std::strlen(buffer));
    }

    auto Terminal::HideCursor() -> void
    {
        Write("\033[?25l", 6);
    }

    auto Terminal::ShowCursor() -> void
    {
        Write("\033[?25h", 6);
    }

#endif
}
