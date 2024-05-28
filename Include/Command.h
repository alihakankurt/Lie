#ifndef __LIE_COMMAND_H__
#define __LIE_COMMAND_H__

#include <Core.h>
#include <Queue.h>

typedef enum CommandKind
{
    COMMAND_NONE = 0,
    COMMAND_PRINT = 1,
    COMMAND_MOVE_CURSOR = 2,
    COMMAND_UPDATE_CURSOR_VISIBILITY = 3,
    COMMAND_CLEAR_SCREEN = 4,
    COMMAND_CLEAR_LINE = 5,
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
    };
} Command;

void MakePrintCommand(Command* command, StringView text);
void MakeMoveCursorCommand(Command* command, u16 x, u16 y);
void MakeHideCursorCommand(Command* command);
void MakeShowCursorCommand(Command* command);
void MakeClearScreenCommand(Command* command, ClearScreenMode value);
void MakeClearLineCommand(Command* command, ClearLineMode value);

DeclareQueue(CommandQueue, Command)

#endif