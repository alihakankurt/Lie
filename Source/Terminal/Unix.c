#include <Terminal.h>

#if defined(LIE_PLATFORM_LINUX) || defined(LIE_PLATFORM_MACOS)

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

struct termios original;
u8 buffer[8];

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

bool ReadInto(usize index)
{
    return read(STDIN_FILENO, &buffer[index], 1) == 1;
}

KeyModifier GetKeyModifiers(u8 value)
{
    value -= 1;

    KeyModifier modifiers = 0;

    if ((value & 0x1) == 0x1)
        modifiers |= ShiftModifier;

    if ((value & 0x2) == 0x2)
        modifiers |= AltModifier;

    if ((value & 0x4) == 0x4)
        modifiers |= CtrlModifier;

    return modifiers;
}

bool ReadKeyModifiers(Event* event, usize index)
{
    if (!ReadInto(index) || !IsDigit(buffer[index]))
        return false;

    u8 value = buffer[index] - '0';
    if (!ReadInto(index))
        return false;

    if (IsDigit(buffer[index]))
    {
        value = value * 10 + buffer[index] - '0';

        if (!ReadInto(index))
            return false;
    }

    event->Key.Modifiers = GetKeyModifiers(value);
    return true;
}

bool HandleSS3Codes(Event* event)
{
    if (!ReadInto(0))
        return false;

    if (buffer[0] == '1' && (!ReadInto(0) || buffer[0] != ';' || !ReadKeyModifiers(event, 0)))
        return false;

    event->Kind = KeyEvent;
    switch (buffer[0])
    {
        case 'A':
            event->Key.Code = UpKey;
            return true;
        case 'B':
            event->Key.Code = DownKey;
            return true;
        case 'C':
            event->Key.Code = RightKey;
            return true;
        case 'D':
            event->Key.Code = LeftKey;
            return true;
        case 'F':
            event->Key.Code = EndKey;
            return true;
        case 'H':
            event->Key.Code = HomeKey;
            return true;
        case 'P':
            event->Key.Code = FunctionKey;
            event->Key.Value = 1;
            return true;
        case 'Q':
            event->Key.Code = FunctionKey;
            event->Key.Value = 2;
            return true;
        case 'R':
            event->Key.Code = FunctionKey;
            event->Key.Value = 3;
            return true;
        case 'S':
            event->Key.Code = FunctionKey;
            event->Key.Value = 4;
            return true;
        default:
            printf("Unknown SS3 code (%d): %d\r\n", __LINE__, buffer[1]);
            return false;
    }
}

bool HandleVTCodes(Event* event)
{
    event->Kind = KeyEvent;
    switch (buffer[0])
    {
        case '1':
            event->Key.Code = HomeKey;
            return true;
        case '2':
            event->Key.Code = InsertKey;
            return true;
        case '3':
            event->Key.Code = DeleteKey;
            return true;
        case '4':
            event->Key.Code = EndKey;
            return true;
        case '5':
            event->Key.Code = PageUpKey;
            return true;
        case '6':
            event->Key.Code = PageDownKey;
            return true;
        case '7':
            event->Key.Code = HomeKey;
            return true;
        case '8':
            event->Key.Code = EndKey;
            return true;
        default:
            printf("Unknown VT code (%d): %d\r\n", __LINE__, buffer[0]);
            return false;
    }
}

bool HandleVTFunctionCodes(Event* event)
{
    event->Kind = KeyEvent;
    event->Key.Code = FunctionKey;
    switch (buffer[0])
    {
        case '1':
            switch (buffer[1])
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
                    printf("Unknown VT Function code (%d): %d\r\n", __LINE__, buffer[1]);
                    return false;
            }

        case '2':
            switch (buffer[1])
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
                    printf("Unknown VT Function code (%d): %d\r\n", __LINE__, buffer[1]);
                    return false;
            }

        case '3':
            switch (buffer[1])
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
                    printf("Unknown VT Function code (%d): %d\r\n", __LINE__, buffer[1]);
                    return false;
            }
        default:
            printf("Unknown VT Function code (%d): %d\r\n", __LINE__, buffer[0]);
            return false;
    }
}

bool HandleXTermCode(Event* event)
{
    event->Kind = KeyEvent;
    switch (buffer[0])
    {
        case 'A':
            event->Key.Code = UpKey;
            return true;
        case 'B':
            event->Key.Code = DownKey;
            return true;
        case 'C':
            event->Key.Code = RightKey;
            return true;
        case 'D':
            event->Key.Code = LeftKey;
            return true;
        case 'F':
            event->Key.Code = EndKey;
            return true;
        case 'H':
            event->Key.Code = HomeKey;
            return true;
        case 'P':
            event->Key.Code = FunctionKey;
            event->Key.Value = 1;
            return true;
        case 'Q':
            event->Key.Code = FunctionKey;
            event->Key.Value = 2;
            return true;
        case 'R':
            event->Key.Code = FunctionKey;
            event->Key.Value = 3;
            return true;
        case 'S':
            event->Key.Code = FunctionKey;
            event->Key.Value = 4;
            return true;
        case 'Z':
            event->Key.Code = TabKey;
            event->Key.Modifiers |= ShiftModifier;
            return true;
        default:
            printf("Unknown XTerm code (%d): %d\r\n", __LINE__, buffer[0]);
            return false;
    }
}

bool HandleCSICodes(Event* event)
{
    if (!ReadInto(0))
        return false;

    if (IsUppercase(buffer[0]))
        return HandleXTermCode(event);

    if (!IsDigit(buffer[0]))
    {
        printf("Unknown CSI code (%d): %d\r\n", __LINE__, buffer[0]);
        return false;
    }

    if (!ReadInto(1))
        return false;

    if (buffer[1] == '~')
        return HandleVTCodes(event);

    if (buffer[1] == ';')
    {
        if (!ReadKeyModifiers(event, 2))
            return false;

        if (buffer[2] == '~')
            return HandleVTCodes(event);

        if (IsUppercase(buffer[2]) && buffer[0] == '1')
        {
            buffer[0] = buffer[2];
            return HandleXTermCode(event);
        }

        printf("Unknown CSI code (%d): %d\r\n", __LINE__, buffer[2]);
        return false;
    }
    else if (IsDigit(buffer[1]))
    {
        if (!ReadInto(2))
            return false;

        if (buffer[2] == '~')
            return HandleVTFunctionCodes(event);

        if (buffer[2] == ';')
        {
            if (!ReadKeyModifiers(event, 2))
                return false;

            if (buffer[2] == '~')
                return HandleVTFunctionCodes(event);

            printf("Unknown CSI code (%d): %d\r\n", __LINE__, buffer[2]);
            return false;
        }

        if (IsUppercase(buffer[2]))
        {
            u8 value = (buffer[0] - '0') * 10 + buffer[1] - '0';
            event->Key.Modifiers = GetKeyModifiers(value);
            return HandleXTermCode(event);
        }

        printf("Unknown CSI code (%d): %d\r\n", __LINE__, buffer[2]);
        return false;
    }

    if (IsUppercase(buffer[1]))
    {
        u8 value = buffer[0] - '0';
        event->Key.Modifiers = GetKeyModifiers(value);
        return HandleXTermCode(event);
    }

    printf("Unknown CSI code (%d): %d\r\n", __LINE__, buffer[1]);
    return false;
}

bool HandleEscapeCode(Event* event)
{
    if (!ReadInto(0) || buffer[0] == '\x1B')
    {
        event->Kind = KeyEvent;
        event->Key.Code = EscapeKey;
        return true;
    }

    if (buffer[0] == 'O')
        return HandleSS3Codes(event);

    if (buffer[0] == '[')
        return HandleCSICodes(event);

    if (buffer[0] == 'b')
    {
        event->Kind = KeyEvent;
        event->Key.Code = LeftKey;
#if defined(LIE_PLATFORM_LINUX)
        event->Key.Modifiers = CtrlModifier;
#elif defined(LIE_PLATFORM_MACOS)
        event->Key.Modifiers = AltModifier;
#endif
        return true;
    }

    if (buffer[0] == 'f')
    {
        event->Kind = KeyEvent;
        event->Key.Code = RightKey;
#if defined(LIE_PLATFORM_LINUX)
        event->Key.Modifiers = CtrlModifier;
#elif defined(LIE_PLATFORM_MACOS)
        event->Key.Modifiers = AltModifier;
#endif
        return true;
    }

    printf("Unknown escape code (%d): %d\r\n", __LINE__, buffer[0]);
    return false;
}

bool PollEvent(Event* event)
{
    *event = (Event){0};

    if (!ReadInto(0))
        return false;

    if (buffer[0] == '\x1B')
        return HandleEscapeCode(event);

    event->Kind = KeyEvent;

    if (buffer[0] == '\xD')
    {
        event->Key.Code = EnterKey;
        return true;
    }

    if (buffer[0] == '\x9')
    {
        event->Key.Code = TabKey;
        return true;
    }

    if (buffer[0] == '\x7F')
    {
        event->Key.Code = BackspaceKey;
        return true;
    }

    event->Key.Code = Character;
    event->Key.Value = buffer[0];

    if ('\x01' <= buffer[0] && buffer[0] <= '\x1F')
    {
        event->Key.Value ^= 64;
        event->Key.Modifiers |= CtrlModifier;
    }

    if (IsUppercase(buffer[0]))
        event->Key.Modifiers |= ShiftModifier;

    return true;
}

void ProcessCommandQueue(CommandQueue* queue)
{
    if (queue->Count == 0)
        return;

    for (Command* command = queue->Items; command < queue->Items + queue->Count; ++command)
    {
        switch (command->Kind)
        {
            case PrintCommand:
                write(STDOUT_FILENO, command->Print.Data, command->Print.Length);
                break;
            case MoveCursorCommand:
                sprintf((char*)buffer, "\x1B[%hu;%huH", command->MoveCursor.Y, command->MoveCursor.X);
                write(STDOUT_FILENO, buffer, strlen((char*)buffer));
                break;
            case ClearScreenCommand:
                sprintf((char*)buffer, "\x1B[%huJ", command->ClearScreen.Value);
                write(STDOUT_FILENO, buffer, strlen((char*)buffer));
                break;
            case ClearLineCommand:
                sprintf((char*)buffer, "\x1B[%huK", command->ClearLine.Value);
                write(STDOUT_FILENO, buffer, strlen((char*)buffer));
                break;
        }
    }
}

#endif