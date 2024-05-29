#include <Terminal.h>

#if defined(LIE_PLATFORM_LINUX) || defined(LIE_PLATFORM_MACOS)

#include <Utility.h>

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>

bool ReadByte(u8* in);
bool WriteStringView(StringView out);

KeyModifier GetKeyModifiers(u8 value);
bool ReadKeyModifiers(Terminal* terminal, KeyModifier* modifiers, usize index);
bool HandleSS3Codes(Terminal* terminal, Event* event, KeyModifier modifiers);
bool HandleXTermCodes(Terminal* terminal, Event* event, KeyModifier modifiers);
bool HandleVTCodes(Terminal* terminal, Event* event, KeyModifier modifiers);
bool HandleVTFunctionCodes(Terminal* terminal, Event* event, KeyModifier modifiers);
bool HandleCSICodes(Terminal* terminal, Event* event);
bool HandleEscapeCodes(Terminal* terminal, Event* event);

struct Terminal
{
    struct termios OriginalTermios;

    u8 In[32];
    String Out;
};

Terminal* CreateTerminal()
{
    Terminal* terminal = (Terminal*)MemoryAllocate(sizeof(Terminal));
    InitializeString(&terminal->Out);
    return terminal;
}

void DestroyTerminal(Terminal* terminal)
{
    FinalizeString(&terminal->Out);
    MemoryFree(terminal);
}

void EnableRawMode(Terminal* terminal)
{
    tcgetattr(STDIN_FILENO, &terminal->OriginalTermios);
    struct termios raw = terminal->OriginalTermios;
    raw.c_iflag &= ~(tcflag_t)(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
    raw.c_oflag &= ~(tcflag_t)(OPOST);
    raw.c_cflag |= (tcflag_t)(CS8);
    raw.c_lflag &= ~(tcflag_t)(ECHO | ISIG | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void DisableRawMode(Terminal* terminal)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal->OriginalTermios);
}

void EnterAlternateScreen(Terminal* terminal)
{
    static StringView enterCode = {.Content = "\x1B[?1049h", .Length = 8};
    WriteStringView(enterCode);
}

void LeaveAlternateScreen(Terminal* terminal)
{
    static StringView leaveCode = {.Content = "\x1B[?1049l", .Length = 8};
    WriteStringView(leaveCode);
}

bool GetTerminalSize(Terminal* terminal, u16* width, u16* height)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
    {
        *width = ws.ws_col;
        *height = ws.ws_row;
        return true;
    }

    static StringView saveCursor = {.Length = 3, .Content = "\x1B[s"};
    static StringView moveCursor = {.Length = 4, .Content = "\x1B[6n"};
    static StringView restoreCursor = {.Length = 3, .Content = "\x1B[u"};

    return WriteStringView(saveCursor) && WriteStringView(moveCursor) && GetCursorPosition(terminal, width, height) && WriteStringView(restoreCursor);
}

bool GetCursorPosition(Terminal* terminal, u16* x, u16* y)
{
    static StringView queryCursor = {.Length = 4, .Content = "\x1B[6n"};
    if (!WriteStringView(queryCursor))
        return false;

    usize index = 0;
    usize semicolon = 0;

    while (index <= 31 && ReadByte(&terminal->In[index]) && terminal->In[index] != 'R')
    {
        if (terminal->In[index] == ';')
            semicolon = index;

        index += 1;
    }

    if (terminal->In[0] != '\x1B' || terminal->In[1] != '[')
        return false;

    StringView yString = {.Content = &terminal->In[2], .Length = semicolon - 2};
    StringView xString = {.Content = &terminal->In[semicolon + 1], .Length = index - semicolon - 1};

    *y = StringViewToUInt16(yString);
    *x = StringViewToUInt16(xString);

    return true;
}

bool ReadEvent(Terminal* terminal, Event* event)
{
    if (!ReadByte(&terminal->In[0]))
        return false;

    if (terminal->In[0] == '\x1B')
        return HandleEscapeCodes(terminal, event);

    if (terminal->In[0] == '\xD')
    {
        MakeKeyEvent(event, KEY_CODE_ENTER, KEY_MODIFIER_NONE, 0);
        return true;
    }

    if (terminal->In[0] == '\x9')
    {
        MakeKeyEvent(event, KEY_CODE_TAB, KEY_MODIFIER_NONE, 0);
        return true;
    }

    if (terminal->In[0] == '\x7F')
    {
        MakeKeyEvent(event, KEY_CODE_BACKSPACE, KEY_MODIFIER_NONE, 0);
        return true;
    }

    KeyModifier modifiers = KEY_MODIFIER_NONE;

    if ('\x01' <= terminal->In[0] && terminal->In[0] <= '\x1F')
    {
        terminal->In[0] ^= 64;
        modifiers |= KEY_MODIFIER_CONTROL;
    }

    if (IsUppercase(terminal->In[0]))
        modifiers |= KEY_MODIFIER_SHIFT;

    MakeKeyEvent(event, KEY_CODE_CHARACTER, modifiers, terminal->In[0]);
    return true;
}

void ProcessCommandQueue(Terminal* terminal, CommandQueue* queue)
{
    Command command;
    while (DequeueCommandQueue(queue, &command))
    {
        switch (command.Kind)
        {
            case COMMAND_NONE:
                break;
            case COMMAND_PRINT:
                AppendStringView(&terminal->Out, command.Print.Text);
                break;
            case COMMAND_MOVE_CURSOR:
                AppendStr(&terminal->Out, "\x1B[");
                AppendStringView(&terminal->Out, UInt16ToStringView(command.MoveCursor.Y));
                AppendChar(&terminal->Out, ';');
                AppendStringView(&terminal->Out, UInt16ToStringView(command.MoveCursor.X));
                AppendChar(&terminal->Out, 'H');
                break;
            case COMMAND_UPDATE_CURSOR_VISIBILITY:
                AppendStr(&terminal->Out, "\x1B[?25");
                AppendChar(&terminal->Out, command.UpdateCursorVisibility.Visible ? 'h' : 'l');
                break;
            case COMMAND_CLEAR_SCREEN:
                AppendStr(&terminal->Out, "\x1B[");
                AppendStringView(&terminal->Out, UInt16ToStringView((u16)command.ClearScreen.Mode));
                AppendChar(&terminal->Out, 'J');
                break;
            case COMMAND_CLEAR_LINE:
                AppendStr(&terminal->Out, "\x1B[");
                AppendStringView(&terminal->Out, UInt16ToStringView((u16)command.ClearLine.Mode));
                AppendChar(&terminal->Out, 'K');
                break;
        }
    }

    StringView out = MakeStringView(&terminal->Out, 0, terminal->Out.Length);
    WriteStringView(out);
    terminal->Out.Length = 0;
}

bool ReadByte(u8* in)
{
    return read(STDIN_FILENO, in, 1) == 1;
}

bool WriteStringView(StringView out)
{
    isize result = write(STDOUT_FILENO, out.Content, out.Length);
    return result >= 0 && (usize)result == out.Length;
}

KeyModifier GetKeyModifiers(u8 value)
{
    value -= 1;

    KeyModifier modifiers = 0;

    if ((value & 0x1) == 0x1)
        modifiers |= KEY_MODIFIER_SHIFT;

    if ((value & 0x2) == 0x2)
        modifiers |= KEY_MODIFIER_ALT;

    if ((value & 0x4) == 0x4)
        modifiers |= KEY_MODIFIER_CONTROL;

    return modifiers;
}

bool ReadKeyModifiers(Terminal* terminal, KeyModifier* modifiers, usize index)
{
    if (!ReadByte(&terminal->In[index]) || !IsDigit(terminal->In[index]))
        return false;

    u8 value = terminal->In[index] - '0';
    if (!ReadByte(&terminal->In[index]))
        return false;

    if (IsDigit(terminal->In[index]))
    {
        value = value * 10 + terminal->In[index] - '0';

        if (!ReadByte(&terminal->In[index]))
            return false;
    }

    *modifiers |= GetKeyModifiers(value);
    return true;
}

bool HandleSS3Codes(Terminal* terminal, Event* event, KeyModifier modifiers)
{
    if (!ReadByte(&terminal->In[0]))
        return false;

    if (terminal->In[0] == '1' && (!ReadByte(&terminal->In[0]) || terminal->In[0] != ';' || !ReadKeyModifiers(terminal, &modifiers, 0)))
        return false;

    return HandleXTermCodes(terminal, event, modifiers);
}

bool HandleXTermCodes(Terminal* terminal, Event* event, KeyModifier modifiers)
{
    switch (terminal->In[0])
    {
        case 'A':
            MakeKeyEvent(event, KEY_CODE_UP, modifiers, 0);
            return true;
        case 'B':
            MakeKeyEvent(event, KEY_CODE_DOWN, modifiers, 0);
            return true;
        case 'C':
            MakeKeyEvent(event, KEY_CODE_RIGHT, modifiers, 0);
            return true;
        case 'D':
            MakeKeyEvent(event, KEY_CODE_LEFT, modifiers, 0);
            return true;
        case 'F':
            MakeKeyEvent(event, KEY_CODE_END, modifiers, 0);
            return true;
        case 'H':
            MakeKeyEvent(event, KEY_CODE_HOME, modifiers, 0);
            return true;
        case 'P':
            MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 1);
            return true;
        case 'Q':
            MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 2);
            return true;
        case 'R':
            MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 3);
            return true;
        case 'S':
            MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 4);
            return true;
        case 'Z':
            MakeKeyEvent(event, KEY_CODE_TAB, modifiers | KEY_MODIFIER_SHIFT, 0);
            return true;
        default:
            printf("Unknown SS3/XTerm code (%d): %d\r\n", __LINE__, terminal->In[1]);
            return false;
    }
}

bool HandleVTCodes(Terminal* terminal, Event* event, KeyModifier modifiers)
{
    switch (terminal->In[0])
    {
        case '1':
            MakeKeyEvent(event, KEY_CODE_HOME, modifiers, 0);
            return true;
        case '2':
            MakeKeyEvent(event, KEY_CODE_INSERT, modifiers, 0);
            return true;
        case '3':
            MakeKeyEvent(event, KEY_CODE_DELETE, modifiers, 0);
            return true;
        case '4':
            MakeKeyEvent(event, KEY_CODE_END, modifiers, 0);
            return true;
        case '5':
            MakeKeyEvent(event, KEY_CODE_PAGE_UP, modifiers, 0);
            return true;
        case '6':
            MakeKeyEvent(event, KEY_CODE_PAGE_DOWN, modifiers, 0);
            return true;
        case '7':
            MakeKeyEvent(event, KEY_CODE_HOME, modifiers, 0);
            return true;
        case '8':
            MakeKeyEvent(event, KEY_CODE_END, modifiers, 0);
            return true;
        default:
            printf("Unknown VT code (%d): %d\r\n", __LINE__, terminal->In[0]);
            return false;
    }
}

bool HandleVTFunctionCodes(Terminal* terminal, Event* event, KeyModifier modifiers)
{
    switch (terminal->In[0])
    {
        case '1':
            switch (terminal->In[1])
            {
                case '0':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 0);
                    return true;
                case '1':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 1);
                    return true;
                case '2':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 2);
                    return true;
                case '3':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 3);
                    return true;
                case '4':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 4);
                    return true;
                case '5':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 5);
                    return true;
                case '7':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 6);
                    return true;
                case '8':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 7);
                    return true;
                case '9':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 8);
                    return true;
                default:
                    printf("Unknown VT Function code (%d): %d\r\n", __LINE__, terminal->In[1]);
                    return false;
            }

        case '2':
            switch (terminal->In[1])
            {
                case '0':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 9);
                    return true;
                case '1':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 10);
                    return true;
                case '3':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 11);
                    return true;
                case '4':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 12);
                    return true;
                case '5':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 13);
                    return true;
                case '6':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 14);
                    return true;
                case '8':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 15);
                    return true;
                case '9':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 16);
                    return true;
                default:
                    printf("Unknown VT Function code (%d): %d\r\n", __LINE__, terminal->In[1]);
                    return false;
            }

        case '3':
            switch (terminal->In[1])
            {
                case '1':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 17);
                    return true;
                case '2':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 18);
                    return true;
                case '3':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 19);
                    return true;
                case '4':
                    MakeKeyEvent(event, KEY_CODE_FUNCTION, modifiers, 20);
                    return true;
                default:
                    printf("Unknown VT Function code (%d): %d\r\n", __LINE__, terminal->In[1]);
                    return false;
            }

        default:
            printf("Unknown VT Function code (%d): %d\r\n", __LINE__, terminal->In[0]);
            return false;
    }
}

bool HandleCSICodes(Terminal* terminal, Event* event)
{
    if (!ReadByte(&terminal->In[0]))
        return false;

    if (IsUppercase(terminal->In[0]))
        return HandleXTermCodes(terminal, event, KEY_MODIFIER_NONE);

    if (!IsDigit(terminal->In[0]))
    {
        printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[0]);
        return false;
    }

    if (!ReadByte(&terminal->In[1]))
        return false;

    if (terminal->In[1] == '~')
        return HandleVTCodes(terminal, event, KEY_MODIFIER_NONE);

    if (terminal->In[1] == ';')
    {
        KeyModifier modifiers = KEY_MODIFIER_NONE;
        if (!ReadKeyModifiers(terminal, &modifiers, 2))
            return false;

        if (terminal->In[2] == '~')
            return HandleVTCodes(terminal, event, modifiers);

        if (IsUppercase(terminal->In[2]) && terminal->In[0] == '1')
        {
            terminal->In[0] = terminal->In[2];
            return HandleXTermCodes(terminal, event, modifiers);
        }

        printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[2]);
        return false;
    }

    if (IsDigit(terminal->In[1]))
    {
        if (!ReadByte(&terminal->In[2]))
            return false;

        if (terminal->In[2] == '~')
            return HandleVTFunctionCodes(terminal, event, KEY_MODIFIER_NONE);

        if (terminal->In[2] == ';')
        {
            KeyModifier modifiers = KEY_MODIFIER_NONE;
            if (!ReadKeyModifiers(terminal, &modifiers, 2))
                return false;

            if (terminal->In[2] == '~')
                return HandleVTFunctionCodes(terminal, event, modifiers);

            printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[2]);
            return false;
        }

        if (IsUppercase(terminal->In[2]))
        {
            u8 value = (terminal->In[0] - '0') * 10 + terminal->In[1] - '0';
            KeyModifier modifiers = GetKeyModifiers(value);
            return HandleXTermCodes(terminal, event, modifiers);
        }

        printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[2]);
        return false;
    }

    if (IsUppercase(terminal->In[1]))
    {
        u8 value = terminal->In[0] - '0';
        KeyModifier modifiers = GetKeyModifiers(value);
        return HandleXTermCodes(terminal, event, modifiers);
    }

    printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[1]);
    return false;
}

bool HandleEscapeCodes(Terminal* terminal, Event* event)
{
    if (!ReadByte(&terminal->In[0]) || terminal->In[0] == '\x1B')
    {
        MakeKeyEvent(event, KEY_CODE_ESCAPE, KEY_MODIFIER_NONE, 0);
        return true;
    }

    if (terminal->In[0] == 'O')
        return HandleSS3Codes(terminal, event, KEY_MODIFIER_NONE);

    if (terminal->In[0] == '[')
        return HandleCSICodes(terminal, event);

    if (terminal->In[0] == 'b')
    {
#if defined(LIE_PLATFORM_LINUX)
        MakeKeyEvent(event, KEY_CODE_LEFT, KEY_MODIFIER_CONTROL, 0);
#elif defined(LIE_PLATFORM_MACOS)
        MakeKeyEvent(event, KEY_CODE_LEFT, KEY_MODIFIER_SHIFT, 0);
#endif
        return true;
    }

    if (terminal->In[0] == 'f')
    {
#if defined(LIE_PLATFORM_LINUX)
        MakeKeyEvent(event, KEY_CODE_RIGHT, KEY_MODIFIER_CONTROL, 0);
#elif defined(LIE_PLATFORM_MACOS)
        MakeKeyEvent(event, KEY_CODE_RIGHT, KEY_MODIFIER_SHIFT, 0);
#endif
        return true;
    }

    printf("Unknown escape code (%d): %d\r\n", __LINE__, terminal->In[0]);
    return false;
}

#endif