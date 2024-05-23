#ifndef __LIE_COMMAND_H__
#define __LIE_COMMAND_H__

#include <Core.h>
#include <Queue.h>

typedef enum CommandKind
{
    COMMAND_NONE = 0,
    COMMAND_PRINT = 1,
    COMMAND_MOVE_CURSOR = 2,
    COMMAND_CLEAR_SCREEN = 3,
    COMMAND_CLEAR_LINE = 4,
} CommandKind;

typedef enum ClearScreenValue
{
    CLEAR_SCREEN_TO_END = 0,
    CLEAR_SCREEN_TO_BEGIN = 1,
    CLEAR_SCREEN_ENTIRE = 2,
} ClearScreenValue;

typedef enum ClearLineValue
{
    CLEAR_LINE_TO_END = 0,
    CLEAR_LINE_TO_BEGIN = 1,
    CLEAR_LINE_ENTIRE = 2,
} ClearLineValue;

typedef struct Command
{
    CommandKind Kind;
    union
    {
        struct PrintCommandData
        {
            u8* Data;
            usize Length;
        } Print;

        struct MoveCursorCommandData
        {
            u16 X;
            u16 Y;
        } MoveCursor;

        struct ClearScreenCommandData
        {
            ClearScreenValue Value;
        } ClearScreen;

        struct ClearLineCommandData
        {
            ClearLineValue Value;
        } ClearLine;
    };
} Command;

void MakePrintCommand(Command* command, u8* data, usize size);
void MakeMoveCursorCommand(Command* command, u16 x, u16 y);
void MakeClearScreenCommand(Command* command, ClearScreenValue value);
void MakeClearLineCommand(Command* command, ClearLineValue value);

DeclareQueue(CommandQueue, Command)

#endif