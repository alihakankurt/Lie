#include <Core.h>
#include <Terminal.h>
#include <stdio.h>

int main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;

    CommandQueue queue;
    InitializeCommandQueue(&queue);

    Terminal* terminal = CreateTerminal();

    u16 cursorX = 1;
    u16 cursorY = 1;

    EnableRawMode(terminal);

    bool running = true;

    Event event;
    Command command;
    u16 width, height;
    GetTerminalSize(terminal, &width, &height);

    StringView quitMessage = AsStringView("Press 'q' to quit\r\n");
    StringView moveMessage = AsStringView("Use arrow keys to move cursor\r\n");

    while (running)
    {
        MakeHideCursorCommand(&command);
        EnqueueCommandQueue(&queue, command);

        MakeMoveCursorCommand(&command, 1, 1);
        EnqueueCommandQueue(&queue, command);

        for (u16 i = 1; i <= height; i++)
        {
            MakePrintCommand(&command, AsStringView("~"));
            EnqueueCommandQueue(&queue, command);

            if (i < height)
            {
                MakePrintCommand(&command, AsStringView("\r\n"));
                EnqueueCommandQueue(&queue, command);
            }

            MakeClearLineCommand(&command, CLEAR_LINE_TO_END);
            EnqueueCommandQueue(&queue, command);
        }

        MakeMoveCursorCommand(&command, 1, 1);
        EnqueueCommandQueue(&queue, command);

        MakePrintCommand(&command, quitMessage);
        EnqueueCommandQueue(&queue, command);

        MakePrintCommand(&command, moveMessage);
        EnqueueCommandQueue(&queue, command);

        MakeMoveCursorCommand(&command, cursorX, cursorY);
        EnqueueCommandQueue(&queue, command);

        MakeShowCursorCommand(&command);
        EnqueueCommandQueue(&queue, command);

        ProcessCommandQueue(terminal, &queue);
        ClearCommandQueue(&queue);

        if (ReadEvent(terminal, &event))
        {
            if (event.Kind == EVENT_KEY)
            {
                if (event.Key.Code == KEY_CODE_CHARACTER && event.Key.Value == 'q')
                    running = false;

                if (event.Key.Code == KEY_CODE_UP)
                    cursorY = cursorY > 1 ? cursorY - 1 : 1;
                else if (event.Key.Code == KEY_CODE_DOWN)
                    cursorY = cursorY < height ? cursorY + 1 : height;
                else if (event.Key.Code == KEY_CODE_LEFT)
                    cursorX = cursorX > 1 ? cursorX - 1 : 1;
                else if (event.Key.Code == KEY_CODE_RIGHT)
                    cursorX = cursorX < width ? cursorX + 1 : width;
            }
        }
    }

    DisableRawMode(terminal);
    DestroyTerminal(terminal);

    FinalizeCommandQueue(&queue);
    return 0;
}
