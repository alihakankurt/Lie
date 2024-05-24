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

    u8 x = 1;
    u8 y = 1;

    EnableRawMode(terminal);

    bool running = true;

    Event event;
    Command command;
    StringView text = AsStringView("Press 'q' to quit");

    while (running)
    {
        MakeMoveCursorCommand(&command, 1, 1);
        EnqueueCommandQueue(&queue, command);

        MakeClearScreenCommand(&command, CLEAR_SCREEN_ENTIRE);
        EnqueueCommandQueue(&queue, command);

        MakeMoveCursorCommand(&command, 1, 1);
        EnqueueCommandQueue(&queue, command);

        MakePrintCommand(&command, text);
        EnqueueCommandQueue(&queue, command);

        MakeMoveCursorCommand(&command, x, y);
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
                    y--;
                else if (event.Key.Code == KEY_CODE_DOWN)
                    y++;
                else if (event.Key.Code == KEY_CODE_RIGHT)
                    x++;
                else if (event.Key.Code == KEY_CODE_LEFT)
                    x--;
            }
        }
    }

    DisableRawMode(terminal);
    DestroyTerminal(terminal);

    FinalizeCommandQueue(&queue);
    return 0;
}
