#include <Editor.h>

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        return RunEditorWithNoFile() ? 0 : 1;
    }

    StringView filepath = { GetStrLength(argv[1]), (u8*)argv[1] };
    return RunEditorWithFile(filepath) ? 0 : 1;
}
