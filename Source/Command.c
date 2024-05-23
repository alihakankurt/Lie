#include <Command.h>
#include <Utility.h>

void MakePrintCommand(Command* command, u8* data, usize size)
{
    command->Kind = COMMAND_PRINT;
    command->Print.Data = data;
    command->Print.Length = size;
}

void MakeMoveCursorCommand(Command* command, u16 x, u16 y)
{
    command->Kind = COMMAND_MOVE_CURSOR;
    command->MoveCursor.X = x;
    command->MoveCursor.Y = y;
}

void MakeClearScreenCommand(Command* command, ClearScreenValue value)
{
    command->Kind = COMMAND_CLEAR_SCREEN;
    command->ClearScreen.Value = value;
}

void MakeClearLineCommand(Command* command, ClearLineValue value)
{
    command->Kind = COMMAND_CLEAR_LINE;
    command->ClearLine.Value = value;
}

ImplementQueue(CommandQueue, Command)
