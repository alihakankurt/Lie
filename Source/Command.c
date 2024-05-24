#include <Command.h>
#include <Utility.h>

void MakePrintCommand(Command* command, StringView text)
{
    command->Kind = COMMAND_PRINT;
    command->Print.Text = text;
}

void MakeMoveCursorCommand(Command* command, u16 x, u16 y)
{
    command->Kind = COMMAND_MOVE_CURSOR;
    command->MoveCursor.X = x;
    command->MoveCursor.Y = y;
}

void MakeClearScreenCommand(Command* command, ClearScreenMode mode)
{
    command->Kind = COMMAND_CLEAR_SCREEN;
    command->ClearScreen.Mode = mode;
}

void MakeClearLineCommand(Command* command, ClearLineMode mode)
{
    command->Kind = COMMAND_CLEAR_LINE;
    command->ClearLine.Mode = mode;
}

ImplementQueue(CommandQueue, Command)
