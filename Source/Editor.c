#include <Editor.h>

void RefreshScreen(Editor* editor);
void ProcessEvent(Editor* editor, Event* event);

void InitializeEditor(Editor* editor)
{
    editor->Terminal = CreateTerminal();
    InitializeCommandQueue(&editor->Commands);

    GetTerminalSize(editor->Terminal, &editor->Width, &editor->Height);

    editor->CursorX = 1;
    editor->CursorY = 1;
    editor->Running = true;
}

void FinalizeEditor(Editor* editor)
{
    DestroyTerminal(editor->Terminal);
    FinalizeCommandQueue(&editor->Commands);
}

void RunWithEmptyFile(Editor* editor)
{
    Event event;

    EnableRawMode(editor->Terminal);

    while (editor->Running)
    {
        RefreshScreen(editor);
        if (ReadEvent(editor->Terminal, &event))
        {
            ProcessEvent(editor, &event);
        }
    }

    DisableRawMode(editor->Terminal);
}

void RefreshScreen(Editor* editor)
{
    Command command;

    MakeHideCursorCommand(&command);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeMoveCursorCommand(&command, 1, 1);
    EnqueueCommandQueue(&editor->Commands, command);

    for (u16 i = 1; i <= editor->Height; i++)
    {
        MakePrintCommand(&command, AsStringView("~"));
        EnqueueCommandQueue(&editor->Commands, command);

        if (i < editor->Height)
        {
            MakePrintCommand(&command, AsStringView("\r\n"));
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
