#include <Editor.h>
#include <List.h>
#include <IO.h>
#include <Terminal.h>

DeclareList(Rows, String);
ImplementList(Rows, String);

typedef struct Editor
{
    Terminal* Terminal;
    CommandQueue Commands;

    u16 Width;
    u16 Height;

    Rows Rows;
    u16 CursorX;
    u16 CursorY;
    bool Running;
} Editor;

void InitializeEditor(Editor* editor)
{
    editor->Terminal = CreateTerminal();
    InitializeCommandQueue(&editor->Commands);

    GetTerminalSize(editor->Terminal, &editor->Width, &editor->Height);

    InitializeRows(&editor->Rows);
    editor->CursorX = 1;
    editor->CursorY = 1;
    editor->Running = true;
}

void FinalizeEditor(Editor* editor)
{
    for (usize index = 0; index < editor->Rows.Count; index += 1)
        FinalizeString(&editor->Rows.Values[index]);

    FinalizeRows(&editor->Rows);

    DestroyTerminal(editor->Terminal);
    FinalizeCommandQueue(&editor->Commands);
}

void RefreshScreen(Editor* editor);
void ProcessEvent(Editor* editor, Event* event);

void RunEditorWithNoFile()
{
    if (!IsTTY())
    {
        static const char notTTY[36] = "Standard input/output is not a TTY.\n";
        WriteStdOut(notTTY, 36);
        return;
    }

    Editor editor;
    InitializeEditor(&editor);
    EnableRawMode(editor.Terminal);
    EnterAlternateScreen(editor.Terminal);

    Event event;
    while (editor.Running)
    {
        RefreshScreen(&editor);
        if (ReadEvent(editor.Terminal, &event))
        {
            ProcessEvent(&editor, &event);
        }
    }

    LeaveAlternateScreen(editor.Terminal);
    DisableRawMode(editor.Terminal);
    FinalizeEditor(&editor);
}

void RefreshScreen(Editor* editor)
{
    Command command;

    MakeHideCursorCommand(&command);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeMoveCursorCommand(&command, 1, 1);
    EnqueueCommandQueue(&editor->Commands, command);

    for (u16 y = 1; y <= editor->Height; y += 1)
    {
        if (y > editor->Rows.Count)
        {
            static StringView emptyLine = {.Length = 1, .Content = "~"};
            MakePrintCommand(&command, emptyLine);
            EnqueueCommandQueue(&editor->Commands, command);
        }
        else
        {
            MakePrintCommand(&command, AsStringView(&editor->Rows.Values[y - 1]));
            EnqueueCommandQueue(&editor->Commands, command);
        }

        if (y < editor->Height)
        {
            static StringView endLine = {.Length = 2, .Content = "\r\n"};
            MakePrintCommand(&command, endLine);
            EnqueueCommandQueue(&editor->Commands, command);
        }

        MakeClearLineCommand(&command, CLEAR_LINE_TO_END);
        EnqueueCommandQueue(&editor->Commands, command);
    }

    MakeMoveCursorCommand(&command, editor->CursorX, editor->CursorY);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeShowCursorCommand(&command);
    EnqueueCommandQueue(&editor->Commands, command);

    ProcessCommandQueue(editor->Terminal, &editor->Commands);
    ClearCommandQueue(&editor->Commands);
}

void ProcessEvent(Editor* editor, Event* event)
{
    switch (event->Kind)
    {
        case EVENT_NONE:
            break;

        case EVENT_KEY:
        {
            switch (event->Key.Code)
            {
                case KEY_CODE_CHARACTER:
                    if (event->Key.Value == 'q')
                        editor->Running = false;
                    break;

                case KEY_CODE_UP:
                    if (editor->CursorY > 1)
                        editor->CursorY -= 1;
                    break;

                case KEY_CODE_DOWN:
                    if (editor->CursorY < editor->Height)
                        editor->CursorY += 1;
                    break;

                case KEY_CODE_LEFT:
                    if (editor->CursorX > 1)
                        editor->CursorX -= 1;
                    break;

                case KEY_CODE_RIGHT:
                    if (editor->CursorX < editor->Width)
                        editor->CursorX += 1;
                    break;

                default:
                    break;
            }
        }
    }
}
