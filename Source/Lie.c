#include <Core.h>
#include <stdio.h>

#include <Terminal.h>

int main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;

    EnableRawMode();

    bool running = true;

    Event event;
    while (running)
    {
        while (PollEvent(&event))
        {
            if (event.Kind == KeyEvent)
            {
                if (event.Key.Code == Character && event.Key.Value == 'q')
                    running = false;

                printf("KeyEvent { Code: %hu, Value: %hu, Modifiers: %hu }\r\n", event.Key.Code, event.Key.Value, event.Key.Modifiers);
            }
        }
    }

    DisableRawMode();
    return 0;
}
