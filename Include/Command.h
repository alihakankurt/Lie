#ifndef __LIE_COMMAND_H__
#define __LIE_COMMAND_H__

#include <Core.h>
#include <Queue.h>

typedef enum ColorKind
{
    COLOR_KIND_RESET = 0,
    COLOR_KIND_ANSI = 1,
    COLOR_KIND_RGB = 2,
} ColorKind;

typedef struct Color
{
    ColorKind Kind;
    union
    {
        struct
        {
            u8 AnsiValue;
        };

        struct
        {
            u8 Red;
            u8 Green;
            u8 Blue;
        };
    };
} Color;

// clang-format off
#define COLOR_RESET (Color){.Kind = COLOR_KIND_RESET}
#define COLOR_BLACK (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 0}
#define COLOR_RED (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 1}
#define COLOR_GREEN (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 2}
#define COLOR_YELLOW (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 3}
#define COLOR_BLUE (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 4}
#define COLOR_MAGENTA (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 5}
#define COLOR_CYAN (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 6}
#define COLOR_WHITE (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 7}
#define COLOR_BRIGHT_BLACK (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 8}
#define COLOR_BRIGHT_RED (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 9}
#define COLOR_BRIGHT_GREEN (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 10}
#define COLOR_BRIGHT_YELLOW (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 11}
#define COLOR_BRIGHT_BLUE (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 12}
#define COLOR_BRIGHT_MAGENTA (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 13}
#define COLOR_BRIGHT_CYAN (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 14}
#define COLOR_BRIGHT_WHITE (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = 15}
#define COLOR_ANSI(value) (Color){.Kind = COLOR_KIND_ANSI, .AnsiValue = value}
#define COLOR_RGB(r, g, b) (Color){.Kind = COLOR_KIND_RGB, .Red = r, .Green = g, .Blue = b}
// clang-format on

typedef enum CommandKind
{
    COMMAND_NONE = 0,
    COMMAND_PRINT = 1,
    COMMAND_MOVE_CURSOR = 2,
    COMMAND_UPDATE_CURSOR_VISIBILITY = 3,
    COMMAND_CLEAR_SCREEN = 4,
    COMMAND_CLEAR_LINE = 5,
    COMMAND_SET_FOREGROUND = 6,
    COMMAND_SET_BACKGROUND = 7,
} CommandKind;

typedef enum ClearScreenMode
{
    CLEAR_SCREEN_TO_END = 0,
    CLEAR_SCREEN_TO_BEGIN = 1,
    CLEAR_SCREEN_ENTIRE = 2,
} ClearScreenMode;

typedef enum ClearLineMode
{
    CLEAR_LINE_TO_END = 0,
    CLEAR_LINE_TO_BEGIN = 1,
    CLEAR_LINE_ENTIRE = 2,
} ClearLineMode;

typedef struct Command
{
    CommandKind Kind;
    union
    {
        struct PrintCommandData
        {
            StringView Text;
        } Print;

        struct MoveCursorCommandData
        {
            u16 X;
            u16 Y;
        } MoveCursor;

        struct UpdateCursorVisibilityCommandData
        {
            bool Visible;
        } UpdateCursorVisibility;

        struct ClearScreenCommandData
        {
            ClearScreenMode Mode;
        } ClearScreen;

        struct ClearLineCommandData
        {
            ClearLineMode Mode;
        } ClearLine;

        struct SetForegroundColorCommandData
        {
            Color Value;
        } SetForeground;

        struct SetBackgroundColorCommandData
        {
            Color Value;
        } SetBackground;
    };
} Command;

void MakePrintCommand(Command* command, StringView text);
void MakeMoveCursorCommand(Command* command, u16 x, u16 y);
void MakeHideCursorCommand(Command* command);
void MakeShowCursorCommand(Command* command);
void MakeClearScreenCommand(Command* command, ClearScreenMode value);
void MakeClearLineCommand(Command* command, ClearLineMode value);
void MakeSetForegroundCommand(Command* command, Color value);
void MakeSetBackgroundCommand(Command* command, Color value);

DeclareQueue(CommandQueue, Command)

#endif