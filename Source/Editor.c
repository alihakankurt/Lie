#include <Editor.h>

void RefreshScreen(Editor* editor);
void ProcessEvent(Editor* editor, Event* event);

void InitializeEditor(Editor* editor)
{
    editor->Terminal = CreateTerminal();
    InitializeCommandQueue(&editor->Commands);

    GetTerminalSize(editor->Terminal, &editor->Width, &editor->Height);

    editor->Rows = NULL;
    editor->CursorX = 1;
    editor->CursorY = 1;
    editor->Running = true;
}

void FinalizeEditor(Editor* editor)
{
    while (editor->Rows != NULL)
    {
        Row* next = editor->Rows->Next;
        FinalizeString(&editor->Rows->Content);
        MemoryFree(editor->Rows);
        editor->Rows = next;
    }

    DestroyTerminal(editor->Terminal);
    FinalizeCommandQueue(&editor->Commands);
}

void RunWithEmptyFile(Editor* editor)
{
    Event event;

    EnableRawMode(editor->Terminal);
    EnterAlternateScreen(editor->Terminal);

    while (editor->Running)
    {
        RefreshScreen(editor);
        if (ReadEvent(editor->Terminal, &event))
        {
            ProcessEvent(editor, &event);
        }
    }

    LeaveAlternateScreen(editor->Terminal);
    DisableRawMode(editor->Terminal);
}

void RefreshScreen(Editor* editor)
{
    Command command;

    MakeHideCursorCommand(&command);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeMoveCursorCommand(&command, 1, 1);
    EnqueueCommandQueue(&editor->Commands, command);

    Row* row = editor->Rows;
    for (u16 y = 1; y <= editor->Height; y += 1)
    {
        if (row == NULL)
        {
            static StringView emptyLine = {.Length = 1, .Content = "~"};
            MakePrintCommand(&command, emptyLine);
            EnqueueCommandQueue(&editor->Commands, command);
        }
        else
        {
            MakePrintCommand(&command, AsStringView(&row->Content));
            EnqueueCommandQueue(&editor->Commands, command);
            row = row->Next;
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
