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

bool RunEditor(Editor* editor);
void RefreshScreen(Editor* editor);
void ProcessEvent(Editor* editor, Event* event);
bool CreateRowsFromFile(StringView filepath, Rows* rows);

bool RunEditorWithNoFile()
{
    Editor editor;
    InitializeEditor(&editor);
    AddToRows(&editor.Rows, EmptyString);
    bool status = RunEditor(&editor);
    FinalizeEditor(&editor);
    return status;
}

bool RunEditorWithFile(StringView filepath)
{
    Editor editor;
    InitializeEditor(&editor);
    bool status = CreateRowsFromFile(filepath, &editor.Rows) && RunEditor(&editor);
    FinalizeEditor(&editor);
    return status;
}

bool CreateRowsFromFile(StringView filepath, Rows* rows)
{
    String content = EmptyString;
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
            String line = EmptyString;
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

bool RunEditor(Editor* editor)
{
    if (!IsTTY())
    {
        static StringView notTTY = AsStringView("Standard input/output is not a TTY.\n");
        WriteStdOut(notTTY.Content, notTTY.Length);
        return false;
    }

    if (editor->Width == 0 || editor->Height == 0)
    {
        static StringView invalidSize = AsStringView("Invalid terminal size.\n");
        WriteStdOut(invalidSize.Content, invalidSize.Length);
        return false;
    }

    EnableRawMode(editor->Terminal);
    EnterAlternateScreen(editor->Terminal);

    Event event;
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
    return true;
}

void RefreshScreen(Editor* editor)
{
    Command command;

    MakeHideCursorCommand(&command);
    EnqueueCommandQueue(&editor->Commands, command);

    usize index = editor->CursorY + editor->OffsetY - 1;
    usize length = editor->Rows.Values[index].Length;

    u16 offsetX = Min(editor->OffsetX, (u16)length);
    u16 offsetY = editor->OffsetY;

    u16 cursorX = Min(editor->CursorX, (u16)(length - offsetX + 1));
    u16 cursorY = editor->CursorY;

    for (u16 y = 1; y <= editor->Height; y += 1)
    {
        MakeMoveCursorCommand(&command, 1, y);
        EnqueueCommandQueue(&editor->Commands, command);

        index = y + offsetY - 1;

        if (index >= editor->Rows.Count)
        {
            static StringView emptyLine = AsStringView("~");
            MakePrintCommand(&command, emptyLine);
            EnqueueCommandQueue(&editor->Commands, command);
        }
        else if (editor->Rows.Values[index].Length > offsetX)
        {
            usize start = offsetX;
            usize end = Min(start + editor->Width, editor->Rows.Values[index].Length);

            MakePrintCommand(&command, MakeStringView(&editor->Rows.Values[index], start, end));
            EnqueueCommandQueue(&editor->Commands, command);
        }

        MakeClearLineCommand(&command, CLEAR_LINE_TO_END);
        EnqueueCommandQueue(&editor->Commands, command);
    }

    MakeMoveCursorCommand(&command, cursorX, cursorY);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeShowCursorCommand(&command);
    EnqueueCommandQueue(&editor->Commands, command);

    ProcessCommandQueue(editor->Terminal, &editor->Commands);
    ClearCommandQueue(&editor->Commands);
}

void MoveCursorToLineStart(Editor* editor)
{
    editor->CursorX = 1;
    editor->OffsetX = 0;
}

void MoveCursorToLineEnd(Editor* editor)
{
    usize index = editor->CursorY + editor->OffsetY - 1;
    usize length = editor->Rows.Values[index].Length;

    editor->OffsetX = (u16)(length - Min(length, editor->Width - 1));
    editor->CursorX = (u16)(length + 1 - editor->OffsetX);
}

void MoveUp(Editor* editor, u16 count)
{
    u16 move = Min(editor->CursorY - 1, count);
    editor->CursorY -= move;

    u16 offset = Min(editor->OffsetY, count - move);
    editor->OffsetY -= offset;
}

void MoveDown(Editor* editor, u16 count)
{
    u16 remaning = (u16)(editor->Rows.Count - editor->OffsetY);

    u16 move = Min(remaning - editor->CursorY, count);
    move = Min(move, editor->Height - editor->CursorY);
    editor->CursorY += move;

    u16 offset = Min(remaning - Min(remaning, editor->Height), count - move);
    editor->OffsetY += offset;
}

void CorrectCursor(Editor* editor, usize length)
{
    editor->OffsetX = Min(editor->OffsetX, (u16)length);
    editor->CursorX = Min(editor->CursorX, (u16)(length + 1 - editor->OffsetX));
}

void MoveLeft(Editor* editor, u16 count)
{
    usize index = editor->CursorY + editor->OffsetY - 1;
    usize length = editor->Rows.Values[index].Length;
    CorrectCursor(editor, length);

    u16 move = Min(editor->CursorX - 1, count);
    editor->CursorX -= move;

    u16 offset = Min(editor->OffsetX, count - move);
    editor->OffsetX -= offset;

    u16 excess = count - move - offset;
    if (excess > 0 && index > 0)
    {
        MoveUp(editor, 1);
        MoveCursorToLineEnd(editor);
        if (excess > 1)
            MoveLeft(editor, excess - 1);
    }
}

void MoveRight(Editor* editor, u16 count)
{
    usize index = editor->CursorY + editor->OffsetY - 1;
    usize length = editor->Rows.Values[index].Length;
    CorrectCursor(editor, length);

    u16 remaining = (u16)(length + 1 - editor->OffsetX);

    u16 move = Min(remaining - editor->CursorX, count);
    move = Min(move, editor->Width - editor->CursorX);
    editor->CursorX += move;

    u16 offset = Min(remaining - editor->CursorX, count - move);
    editor->OffsetX += offset;

    u16 excess = count - move - offset;
    if (excess > 0 && index < editor->Rows.Count - 1)
    {
        MoveDown(editor, 1);
        MoveCursorToLineStart(editor);
        if (excess > 1)
            MoveRight(editor, excess - 1);
    }
}

void ProcessEvent(Editor* editor, Event* event)
{
    switch (event->Kind)
    {
        case EVENT_NONE:
            break;

        case EVENT_KEY:
            switch (event->Key.Code)
            {
                case KEY_CODE_CHARACTER:
                    if (event->Key.Value == 'q')
                        editor->Running = false;
                    break;

                case KEY_CODE_UP:
                    MoveUp(editor, 1);
                    break;

                case KEY_CODE_DOWN:
                    MoveDown(editor, 1);
                    break;

                case KEY_CODE_LEFT:
                    MoveLeft(editor, 1);
                    break;

                case KEY_CODE_RIGHT:
                    MoveRight(editor, 1);
                    break;

                // FIXME(alihakankurt): Maybe we should create functions for these two
                // which prioritize the offsetting instead of the moving cursor first.
                case KEY_CODE_PAGE_UP:
                    MoveUp(editor, editor->Height);
                    break;

                case KEY_CODE_PAGE_DOWN:
                    MoveDown(editor, editor->Height);
                    break;

                default:
                    break;
            }
            break;
    }
}
