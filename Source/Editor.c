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
    u16 OffsetX;
    u16 OffsetY;
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
    editor->OffsetX = 0;
    editor->OffsetY = 0;
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

bool RunEditor(Rows rows);
void RefreshScreen(Editor* editor);
void ProcessEvent(Editor* editor, Event* event);
bool CreateRowsFromFile(StringView filepath, Rows* rows);

bool RunEditorWithNoFile()
{
    Rows rows;
    InitializeRows(&rows);
    return RunEditor(rows);
}

bool RunEditorWithFile(StringView filepath)
{
    Rows rows;
    return CreateRowsFromFile(filepath, &rows) && RunEditor(rows);
}

bool CreateRowsFromFile(StringView filepath, Rows* rows)
{
    InitializeRows(rows);

    String content;
    InitializeString(&content);

    if (!ReadFile(filepath, &content))
    {
        static const StringView fileError = AsStringView("Failed to read the file.\n");
        WriteStdOut(fileError.Content, fileError.Length);
        return false;
    }

    for (usize start = 0, end = 0; end < content.Length; end += 1)
    {
        if (content.Content[end] == '\n' || content.Content[end] == '\0')
        {
            String line;
            InitializeString(&line);

            StringView view = MakeStringView(&content, start, end);
            if (view.Length > 0)
            {
                if (view.Content[view.Length - 1] == '\r')
                    view.Length -= 1;

                AppendStringView(&line, view);
            }

            AddToRows(rows, line);
            start = end + 1;
        }
    }

    FinalizeString(&content);
    return true;
}

bool RunEditor(Rows rows)
{
    if (!IsTTY())
    {
        static StringView notTTY = AsStringView("Standard input/output is not a TTY.\n");
        WriteStdOut(notTTY.Content, notTTY.Length);
        return false;
    }

    Editor editor;
    InitializeEditor(&editor);
    EnableRawMode(editor.Terminal);
    EnterAlternateScreen(editor.Terminal);

    editor.Rows = rows;

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

    return true;
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
        usize index = y + editor->OffsetY - 1;
        if (index >= editor->Rows.Count)
        {
            static StringView emptyLine = AsStringView("~");
            MakePrintCommand(&command, emptyLine);
            EnqueueCommandQueue(&editor->Commands, command);
        }
        else if (editor->Rows.Values[index].Length > 0)
        {
            usize start = editor->OffsetX;
            usize end = Min(start + editor->Width, editor->Rows.Values[index].Length);

            MakePrintCommand(&command, MakeStringView(&editor->Rows.Values[index], start, end));
            EnqueueCommandQueue(&editor->Commands, command);
        }

        MakeClearLineCommand(&command, CLEAR_LINE_TO_END);
        EnqueueCommandQueue(&editor->Commands, command);

        if (y < editor->Height)
        {
            static StringView endLine = AsStringView("\r\n");
            MakePrintCommand(&command, endLine);
            EnqueueCommandQueue(&editor->Commands, command);
        }
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
                    else if (editor->OffsetY > 0)
                        editor->OffsetY -= 1;
                    break;

                case KEY_CODE_DOWN:
                    if (editor->CursorY < editor->Height)
                        editor->CursorY += 1;
                    else if (editor->CursorY + editor->OffsetY < editor->Rows.Count)
                        editor->OffsetY += 1;
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
