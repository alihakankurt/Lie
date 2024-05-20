#include <Command.h>

#include <stdlib.h>

void InitializeCommandQueue(CommandQueue* queue)
{
    *queue = (CommandQueue){0};
}

void FinalizeCommandQueue(CommandQueue* queue)
{
    free(queue->Items);
    *queue = (CommandQueue){0};
}

void EnqueueCommand(CommandQueue* queue, Command command)
{
    if (queue->Count >= queue->Capacity)
    {
        queue->Capacity = (queue->Capacity == 0) ? 4 : (queue->Capacity << 1);
        queue->Items = (Command*)realloc(queue->Items, queue->Capacity * sizeof(Command));
    }

    queue->Items[queue->Count++] = command;
}

void ClearCommandQueue(CommandQueue* queue)
{
    queue->Count = 0;
}

void EnqueuePrint(CommandQueue* queue, u8* data, usize length)
{
    Command command;
    command.Kind = PrintCommand;
    command.Print.Data = data;
    command.Print.Length = length;

    EnqueueCommand(queue, command);
}

void EnqueueMoveCursor(CommandQueue* queue, u16 x, u16 y)
{
    Command command;
    command.Kind = MoveCursorCommand;
    command.MoveCursor.X = x;
    command.MoveCursor.Y = y;

    EnqueueCommand(queue, command);
}

void EnqueueClearScreen(CommandQueue* queue, ClearScreenValue value)
{
    Command command;
    command.Kind = ClearScreenCommand;
    command.ClearScreen.Value = value;

    EnqueueCommand(queue, command);
}

void EnqueueClearLine(CommandQueue* queue, ClearLineValue value)
{
    Command command;
    command.Kind = ClearLineCommand;
    command.ClearLine.Value = value;

    EnqueueCommand(queue, command);
}
