#include <Terminal.hh>

#if defined(LIE_UNIX)

auto Lie::Terminal::original = termios();

auto Lie::Terminal::EnableRawMode() -> void
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

auto Lie::Terminal::DisableRawMode() -> void
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

auto Lie::Terminal::GetWindowSize() -> Size
{
    winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return { ws.ws_col, ws.ws_row };
}

auto Lie::Terminal::Read() -> char
{
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
        if (nread == -1 && errno != EAGAIN)
            throw std::runtime_error("read");

    return c;
}

auto Lie::Terminal::Write(const void* data, int size) -> bool
{
    return write(STDOUT_FILENO, data, size) == size;
}

#endif
