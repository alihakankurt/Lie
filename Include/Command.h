#ifndef __LIE_COMMAND_H__
#define __LIE_COMMAND_H__

#include <Core.h>

typedef enum CommandKind
{
    PrintCommand = 1,
    MoveCursorCommand = 2,
    ClearScreenCommand = 3,
    ClearLineCommand = 4,
} CommandKind;

typedef enum ClearScreenValue
{
    ClearScreenToEnd = 0,
    ClearScreenToBeginning = 1,
    ClearScreenEntire = 2,
} ClearScreenValue;

typedef enum ClearLineValue
{
    ClearLineToEnd = 0,
    ClearLineToBeginning = 1,
    ClearLineEntire = 2,
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

typedef struct CommandQueue
{
    Command* Items;
    u32 Count;
    u32 Capacity;
} CommandQueue;

void InitializeCommandQueue(CommandQueue* queue);
void FinalizeCommandQueue(CommandQueue* queue);

void EnqueueCommand(CommandQueue* queue, Command command);
void ClearCommandQueue(CommandQueue* queue);

void EnqueuePrint(CommandQueue* queue, u8* data, usize size);
void EnqueueMoveCursor(CommandQueue* queue, u16 x, u16 y);
void EnqueueClearScreen(CommandQueue* queue, ClearScreenValue value);
void EnqueueClearLine(CommandQueue* queue, ClearLineValue value);

#endif