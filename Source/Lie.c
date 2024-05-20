#include <Core.h>
#include <stdio.h>

#include <Terminal.h>

int main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;

    CommandQueue queue;
    InitializeCommandQueue(&queue);

    u8 x = 1;
    u8 y = 1;

    EnableRawMode();

    bool running = true;

    Event event;
    while (running)
    {
        if (PollEvent(&event))
        {
            if (event.Kind == KeyEvent)
            {
                if (event.Key.Code == Character && event.Key.Value == 'q')
                    running = false;

                if (event.Key.Code == UpKey)
                    y--;
                else if (event.Key.Code == DownKey)
                    y++;
                else if (event.Key.Code == RightKey)
                    x++;
                else if (event.Key.Code == LeftKey)
                    x--;

                // printf("KeyEvent { Code: %hu, Value: %hu, Modifiers: %hu }\r\n", event.Key.Code, event.Key.Value, event.Key.Modifiers);
            }
        }

        ClearCommandQueue(&queue);

        EnqueueMoveCursor(&queue, 1, 1);
        EnqueueClearScreen(&queue, ClearScreenEntire);
        EnqueueMoveCursor(&queue, 1, 1);
        EnqueuePrint(&queue, (u8*)"Lie Terminal Test", 17);
        EnqueueMoveCursor(&queue, x, y);

        ProcessCommandQueue(&queue);
    }

    DisableRawMode();

    FinalizeCommandQueue(&queue);
    return 0;
}
