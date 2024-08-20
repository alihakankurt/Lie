#include <Editor.h>

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        return RunEditorWithNoFile() ? 0 : 1;
    }

    String filepath = EmptyString;
    AppendStr(&filepath, argv[1]);
    return RunEditorWithFile(filepath) ? 0 : 1;
}
