#include <Terminal.h>

#if defined(LIE_PLATFORM_LINUX) || defined(LIE_PLATFORM_MACOS)

#include <Utility.h>

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>

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

bool ReadInto(Terminal* terminal, usize index)
{
    return read(STDIN_FILENO, &terminal->In[index], 1) == 1;
}

void WriteOut(Terminal* terminal)
{
    write(STDOUT_FILENO, terminal->Out.Content, terminal->Out.Length);
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

    return write(STDOUT_FILENO, "\x1B[s\x1B[999C\x1B[999B", 18) == 18
        && GetCursorPosition(terminal, width, height)
        && write(STDOUT_FILENO, "\x1B[u", 3) == 3;
}

bool GetCursorPosition(Terminal* terminal, u16* x, u16* y)
{
    if (write(STDOUT_FILENO, "\x1B[6n", 4) != 4)
        return false;

    usize index = 0;
    usize semicolon = 0;

    while (index <= 31 && ReadInto(terminal, index) && terminal->In[index] != 'R')
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

bool ReadKeyModifiers(Terminal* terminal, Event* event, usize index)
{
    if (!ReadInto(terminal, index) || !IsDigit(terminal->In[index]))
        return false;

    u8 value = terminal->In[index] - '0';
    if (!ReadInto(terminal, index))
        return false;

    if (IsDigit(terminal->In[index]))
    {
        value = value * 10 + terminal->In[index] - '0';

        if (!ReadInto(terminal, index))
            return false;
    }

    event->Key.Modifiers = GetKeyModifiers(value);
    return true;
}

bool HandleSS3Codes(Terminal* terminal, Event* event)
{
    if (!ReadInto(terminal, 0))
        return false;

    if (terminal->In[0] == '1' && (!ReadInto(terminal, 0) || terminal->In[0] != ';' || !ReadKeyModifiers(terminal, event, 0)))
        return false;

    event->Kind = EVENT_KEY;
    switch (terminal->In[0])
    {
        case 'A':
            event->Key.Code = KEY_CODE_UP;
            return true;
        case 'B':
            event->Key.Code = KEY_CODE_DOWN;
            return true;
        case 'C':
            event->Key.Code = KEY_CODE_RIGHT;
            return true;
        case 'D':
            event->Key.Code = KEY_CODE_LEFT;
            return true;
        case 'F':
            event->Key.Code = KEY_CODE_END;
            return true;
        case 'H':
            event->Key.Code = KEY_CODE_HOME;
            return true;
        case 'P':
            event->Key.Code = KEY_CODE_FUNCTION;
            event->Key.Value = 1;
            return true;
        case 'Q':
            event->Key.Code = KEY_CODE_FUNCTION;
            event->Key.Value = 2;
            return true;
        case 'R':
            event->Key.Code = KEY_CODE_FUNCTION;
            event->Key.Value = 3;
            return true;
        case 'S':
            event->Key.Code = KEY_CODE_FUNCTION;
            event->Key.Value = 4;
            return true;
        default:
            printf("Unknown SS3 code (%d): %d\r\n", __LINE__, terminal->In[1]);
            return false;
    }
}

bool HandleVTCodes(Terminal* terminal, Event* event)
{
    event->Kind = EVENT_KEY;
    switch (terminal->In[0])
    {
        case '1':
            event->Key.Code = KEY_CODE_HOME;
            return true;
        case '2':
            event->Key.Code = KEY_CODE_INSERT;
            return true;
        case '3':
            event->Key.Code = KEY_CODE_DELETE;
            return true;
        case '4':
            event->Key.Code = KEY_CODE_END;
            return true;
        case '5':
            event->Key.Code = KEY_CODE_PAGE_UP;
            return true;
        case '6':
            event->Key.Code = KEY_CODE_PAGE_DOWN;
            return true;
        case '7':
            event->Key.Code = KEY_CODE_HOME;
            return true;
        case '8':
            event->Key.Code = KEY_CODE_END;
            return true;
        default:
            printf("Unknown VT code (%d): %d\r\n", __LINE__, terminal->In[0]);
            return false;
    }
}

bool HandleVTFunctionCodes(Terminal* terminal, Event* event)
{
    event->Kind = EVENT_KEY;
    event->Key.Code = KEY_CODE_FUNCTION;
    switch (terminal->In[0])
    {
        case '1':
            switch (terminal->In[1])
            {
                case '0':
                    event->Key.Value = 0;
                    return true;
                case '1':
                    event->Key.Value = 1;
                    return true;
                case '2':
                    event->Key.Value = 2;
                    return true;
                case '3':
                    event->Key.Value = 3;
                    return true;
                case '4':
                    event->Key.Value = 4;
                    return true;
                case '5':
                    event->Key.Value = 5;
                    return true;
                case '7':
                    event->Key.Value = 6;
                    return true;
                case '8':
                    event->Key.Value = 7;
                    return true;
                case '9':
                    event->Key.Value = 8;
                    return true;
                default:
                    printf("Unknown VT Function code (%d): %d\r\n", __LINE__, terminal->In[1]);
                    return false;
            }

        case '2':
            switch (terminal->In[1])
            {
                case '0':
                    event->Key.Value = 9;
                    return true;
                case '1':
                    event->Key.Value = 10;
                    return true;
                case '3':
                    event->Key.Value = 11;
                    return true;
                case '4':
                    event->Key.Value = 12;
                    return true;
                case '5':
                    event->Key.Value = 13;
                    return true;
                case '6':
                    event->Key.Value = 14;
                    return true;
                case '8':
                    event->Key.Value = 15;
                    return true;
                case '9':
                    event->Key.Value = 16;
                    return true;
                default:
                    printf("Unknown VT Function code (%d): %d\r\n", __LINE__, terminal->In[1]);
                    return false;
            }

        case '3':
            switch (terminal->In[1])
            {
                case '1':
                    event->Key.Value = 17;
                    return true;
                case '2':
                    event->Key.Value = 18;
                    return true;
                case '3':
                    event->Key.Value = 19;
                    return true;
                case '4':
                    event->Key.Value = 20;
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

bool HandleXTermCode(Terminal* terminal, Event* event)
{
    event->Kind = EVENT_KEY;
    switch (terminal->In[0])
    {
        case 'A':
            event->Key.Code = KEY_CODE_UP;
            return true;
        case 'B':
            event->Key.Code = KEY_CODE_DOWN;
            return true;
        case 'C':
            event->Key.Code = KEY_CODE_RIGHT;
            return true;
        case 'D':
            event->Key.Code = KEY_CODE_LEFT;
            return true;
        case 'F':
            event->Key.Code = KEY_CODE_END;
            return true;
        case 'H':
            event->Key.Code = KEY_CODE_HOME;
            return true;
        case 'P':
            event->Key.Code = KEY_CODE_FUNCTION;
            event->Key.Value = 1;
            return true;
        case 'Q':
            event->Key.Code = KEY_CODE_FUNCTION;
            event->Key.Value = 2;
            return true;
        case 'R':
            event->Key.Code = KEY_CODE_FUNCTION;
            event->Key.Value = 3;
            return true;
        case 'S':
            event->Key.Code = KEY_CODE_FUNCTION;
            event->Key.Value = 4;
            return true;
        case 'Z':
            event->Key.Code = KEY_CODE_TAB;
            event->Key.Modifiers |= KEY_MODIFIER_SHIFT;
            return true;
        default:
            printf("Unknown XTerm code (%d): %d\r\n", __LINE__, terminal->In[0]);
            return false;
    }
}

bool HandleCSICodes(Terminal* terminal, Event* event)
{
    if (!ReadInto(terminal, 0))
        return false;

    if (IsUppercase(terminal->In[0]))
        return HandleXTermCode(terminal, event);

    if (!IsDigit(terminal->In[0]))
    {
        printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[0]);
        return false;
    }

    if (!ReadInto(terminal, 1))
        return false;

    if (terminal->In[1] == '~')
        return HandleVTCodes(terminal, event);

    if (terminal->In[1] == ';')
    {
        if (!ReadKeyModifiers(terminal, event, 2))
            return false;

        if (terminal->In[2] == '~')
            return HandleVTCodes(terminal, event);

        if (IsUppercase(terminal->In[2]) && terminal->In[0] == '1')
        {
            terminal->In[0] = terminal->In[2];
            return HandleXTermCode(terminal, event);
        }

        printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[2]);
        return false;
    }
    else if (IsDigit(terminal->In[1]))
    {
        if (!ReadInto(terminal, 2))
            return false;

        if (terminal->In[2] == '~')
            return HandleVTFunctionCodes(terminal, event);

        if (terminal->In[2] == ';')
        {
            if (!ReadKeyModifiers(terminal, event, 2))
                return false;

            if (terminal->In[2] == '~')
                return HandleVTFunctionCodes(terminal, event);

            printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[2]);
            return false;
        }

        if (IsUppercase(terminal->In[2]))
        {
            u8 value = (terminal->In[0] - '0') * 10 + terminal->In[1] - '0';
            event->Key.Modifiers = GetKeyModifiers(value);
            return HandleXTermCode(terminal, event);
        }

        printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[2]);
        return false;
    }

    if (IsUppercase(terminal->In[1]))
    {
        u8 value = terminal->In[0] - '0';
        event->Key.Modifiers = GetKeyModifiers(value);
        return HandleXTermCode(terminal, event);
    }

    printf("Unknown CSI code (%d): %d\r\n", __LINE__, terminal->In[1]);
    return false;
}

bool HandleEscapeCode(Terminal* terminal, Event* event)
{
    if (!ReadInto(terminal, 0) || terminal->In[0] == '\x1B')
    {
        event->Kind = EVENT_KEY;
        event->Key.Code = KEY_CODE_ESCAPE;
        return true;
    }

    if (terminal->In[0] == 'O')
        return HandleSS3Codes(terminal, event);

    if (terminal->In[0] == '[')
        return HandleCSICodes(terminal, event);

    if (terminal->In[0] == 'b')
    {
        event->Kind = EVENT_KEY;
        event->Key.Code = KEY_CODE_LEFT;
#if defined(LIE_PLATFORM_LINUX)
        event->Key.Modifiers = KEY_MODIFIER_CONTROL;
#elif defined(LIE_PLATFORM_MACOS)
        event->Key.Modifiers = KEY_MODIFIER_CONTROL;
#endif
        return true;
    }

    if (terminal->In[0] == 'f')
    {
        event->Kind = EVENT_KEY;
        event->Key.Code = KEY_CODE_RIGHT;
#if defined(LIE_PLATFORM_LINUX)
        event->Key.Modifiers = KEY_MODIFIER_CONTROL;
#elif defined(LIE_PLATFORM_MACOS)
        event->Key.Modifiers = KEY_MODIFIER_CONTROL;
#endif
        return true;
    }

    printf("Unknown escape code (%d): %d\r\n", __LINE__, terminal->In[0]);
    return false;
}

bool ReadEvent(Terminal* terminal, Event* event)
{
    *event = (Event){0};

    if (!ReadInto(terminal, 0))
        return false;

    if (terminal->In[0] == '\x1B')
        return HandleEscapeCode(terminal, event);

    event->Kind = EVENT_KEY;

    if (terminal->In[0] == '\xD')
    {
        event->Key.Code = KEY_CODE_ENTER;
        return true;
    }

    if (terminal->In[0] == '\x9')
    {
        event->Key.Code = KEY_CODE_TAB;
        return true;
    }

    if (terminal->In[0] == '\x7F')
    {
        event->Key.Code = KEY_CODE_BACKSPACE;
        return true;
    }

    event->Key.Code = KEY_CODE_CHARACTER;
    event->Key.Value = terminal->In[0];

    if ('\x01' <= terminal->In[0] && terminal->In[0] <= '\x1F')
    {
        event->Key.Value ^= 64;
        event->Key.Modifiers |= KEY_MODIFIER_CONTROL;
    }

    if (IsUppercase(terminal->In[0]))
        event->Key.Modifiers |= KEY_MODIFIER_SHIFT;

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

    WriteOut(terminal);
    terminal->Out.Length = 0;
}

#endif