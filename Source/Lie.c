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
            if (event.Kind == EK_Key)
            {
                if (event.Key.Code == 'q')
                {
                    running = false;
                }

                printf("Key: %d\r\n", event.Key.Code);
            }
        }
    }

    DisableRawMode();
    return 0;
}
