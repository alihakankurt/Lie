#include <Terminal.h>

#if defined(LIE_PLATFORM_LINUX) || defined(LIE_PLATFORM_MACOS)

#include <termios.h>
#include <unistd.h>

struct termios original;

void EnableRawMode()
{
    tcgetattr(STDIN_FILENO, &original);
    struct termios raw = original;
    raw.c_iflag &= ~(tcflag_t)(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
    raw.c_oflag &= ~(tcflag_t)(OPOST);
    raw.c_cflag |= (tcflag_t)(CS8);
    raw.c_lflag &= ~(tcflag_t)(ECHO | ISIG | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void DisableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

bool PollEvent(Event* event)
{
    char c;
    isize n = read(STDIN_FILENO, &c, 1);

    if (n <= 0)
    {
        return false;
    }

    if (c != '\x1b')
    {
        event->Kind = EK_Key;
        event->Key.Code = c;
        return true;
    }

    return false;
}

#endif