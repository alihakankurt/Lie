#include <Editor.h>

int main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;

    Editor editor;
    InitializeEditor(&editor);

    RunWithEmptyFile(&editor);

    FinalizeEditor(&editor);

    return 0;
}
