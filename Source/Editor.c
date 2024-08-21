#include <Editor.h>
#include <List.h>
#include <IO.h>
#include <Terminal.h>

DeclareList(Rows, String);
ImplementList(Rows, String);

typedef enum EditorMode
{
    EDITOR_MODE_VIEW,
    EDITOR_MODE_EDIT,
} EditorMode;

typedef struct Editor
{
    Terminal* Terminal;
    CommandQueue Commands;

    Rows Rows;
    String Filepath;

    bool Running;
    EditorMode Mode;

    u16 Width;
    u16 Height;

    u16 CursorX;
    u16 CursorY;

    u16 FixedCursorX;
    u16 FixedCursorY;

    u16 OffsetX;
    u16 OffsetY;

    String Status;
    u16 StatusTimeout;
    bool IsErrorStatus;
} Editor;

void InitializeEditor(Editor* editor)
{
    editor->Terminal = CreateTerminal();
    InitializeCommandQueue(&editor->Commands);

    InitializeRows(&editor->Rows);
    editor->Filepath = EmptyString;

    editor->Running = true;
    editor->Mode = EDITOR_MODE_VIEW;

    GetTerminalSize(editor->Terminal, &editor->Width, &editor->Height);

    editor->CursorX = 1;
    editor->CursorY = 1;

    editor->FixedCursorX = 1;
    editor->FixedCursorY = 1;

    editor->OffsetX = 0;
    editor->OffsetY = 0;

    InitializeString(&editor->Status);
    editor->StatusTimeout = 0;
    editor->IsErrorStatus = false;
}

void FinalizeEditor(Editor* editor)
{
    FinalizeString(&editor->Status);

    FinalizeString(&editor->Filepath);
    for (usize index = 0; index < editor->Rows.Count; index += 1)
        FinalizeString(&editor->Rows.Values[index]);

    FinalizeRows(&editor->Rows);

    DestroyTerminal(editor->Terminal);
    FinalizeCommandQueue(&editor->Commands);
}

void PrepareStatusMessage(Editor* editor, StringView message, bool isError)
{
    editor->Status.Length = 0;
    AppendStringView(&editor->Status, message);
    editor->StatusTimeout = 10;
    editor->IsErrorStatus = isError;
}

void SaveFile(Editor* editor);
bool CreateRowsFromFile(Editor* editor);
bool RunEditor(Editor* editor);
void FixCursorPosition(Editor* editor);
void RefreshScreen(Editor* editor);
void ProcessEvent(Editor* editor, Event* event);
bool EditorPrompt(Editor* editor, String* prompt, StringView* out);

bool RunEditorWithNoFile()
{
    Editor editor;
    InitializeEditor(&editor);
    AddToRows(&editor.Rows, EmptyString);
    bool status = RunEditor(&editor);
    FinalizeEditor(&editor);
    return status;
}

bool RunEditorWithFile(String filepath)
{
    Editor editor;
    InitializeEditor(&editor);
    editor.Filepath = filepath;
    bool status = CreateRowsFromFile(&editor) && RunEditor(&editor);
    FinalizeEditor(&editor);
    return status;
}

void SaveFile(Editor* editor)
{
    if (editor->Filepath.Length == 0)
    {
        String prompt = EmptyString;
        AppendStringView(&prompt, AsStringView("Save as: "));
        StringView out = EmptyStringView;
        if (!EditorPrompt(editor, &prompt, &out))
        {
            FinalizeString(&prompt);
            return;
        }

        if (out.Length == 0)
        {
            static const StringView fileError = AsStringView("Invalid file name.");
            PrepareStatusMessage(editor, fileError, true);
            FinalizeString(&prompt);
            return;
        }

        editor->Filepath.Length = 0;
        AppendStringView(&editor->Filepath, out);
        FinalizeString(&prompt);
    }

    String content = EmptyString;
    for (usize index = 0; index < editor->Rows.Count; index += 1)
    {
        String* current = &editor->Rows.Values[index];
        if (current->Length > 0)
        {
            AppendString(&content, &editor->Rows.Values[index]);
        }

        if (index < editor->Rows.Count - 1)
        {
            AppendChar(&content, '\n');
        }
    }

    if (WriteFile(ToStringView(&editor->Filepath), ToStringView(&content)))
    {
        static const StringView fileSaved = AsStringView("The content saved to the file.");
        PrepareStatusMessage(editor, fileSaved, false);
    }
    else
    {
        static const StringView fileError = AsStringView("Failed to write the file.");
        PrepareStatusMessage(editor, fileError, true);
    }

    FinalizeString(&content);
}

bool CreateRowsFromFile(Editor* editor)
{
    String content = EmptyString;
    if (!ReadFile(ToStringView(&editor->Filepath), &content))
    {
        static const StringView fileError = AsStringView("Failed to read the file.\n");
        WriteStdOut(fileError.Content, fileError.Length);
        return false;
    }

    for (usize start = 0, end = 0; end <= content.Length; end += 1)
    {
        if (content.Content[end] == '\n' || end == content.Length)
        {
            String line = EmptyString;
            StringView view = MakeStringView(&content, start, end);
            if (view.Length > 0)
            {
                if (view.Content[view.Length - 1] == '\r')
                    view.Length -= 1;

                AppendStringView(&line, view);
            }

            AddToRows(&editor->Rows, line);
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
        FixCursorPosition(editor);
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

void FixCursorPosition(Editor* editor)
{
    usize index = editor->CursorY - 1 + editor->OffsetY;
    usize length = editor->Rows.Values[index].Length;

    editor->FixedCursorX = Min(editor->CursorX, (u16)(length + 1 - editor->OffsetX));
    editor->FixedCursorY = editor->CursorY;
}

void PrintLines(Editor* editor)
{
    Command command;

    for (u16 height = 1; height < editor->Height; height += 1)
    {
        MakeMoveCursorCommand(&command, 1, height);
        EnqueueCommandQueue(&editor->Commands, command);

        usize rowIndex = height - 1 + editor->OffsetY;

        if (rowIndex < editor->Rows.Count)
        {
            String* currentRow = &editor->Rows.Values[rowIndex];
            usize startIndex = Min(editor->OffsetX, currentRow->Length);
            usize endIndex = Min(startIndex + editor->Width, currentRow->Length);
            StringView contentToWrite = MakeStringView(currentRow, startIndex, endIndex);
            if (contentToWrite.Length > 0)
            {
                MakePrintCommand(&command, contentToWrite);
                EnqueueCommandQueue(&editor->Commands, command);
            }
        }
        else
        {
            static StringView emptyLine = AsStringView("~");
            MakePrintCommand(&command, emptyLine);
            EnqueueCommandQueue(&editor->Commands, command);
        }

        MakeClearLineCommand(&command, CLEAR_LINE_TO_END);
        EnqueueCommandQueue(&editor->Commands, command);
    }
}

void PrintStatusMessage(Editor* editor)
{
    Command command;

    MakeMoveCursorCommand(&command, 1, editor->Height);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeSetForegroundCommand(&command, COLOR_WHITE);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeSetBackgroundCommand(&command, editor->IsErrorStatus ? COLOR_RED : COLOR_CYAN);
    EnqueueCommandQueue(&editor->Commands, command);

    MakePrintCommand(&command, ToStringView(&editor->Status));
    EnqueueCommandQueue(&editor->Commands, command);

    MakeClearLineCommand(&command, CLEAR_LINE_TO_END);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeSetForegroundCommand(&command, COLOR_RESET);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeSetBackgroundCommand(&command, COLOR_RESET);
    EnqueueCommandQueue(&editor->Commands, command);
}

void PrintEditorInfo(Editor* editor)
{
    Command command;

    MakeMoveCursorCommand(&command, 1, editor->Height);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeSetForegroundCommand(&command, COLOR_BLACK);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeSetBackgroundCommand(&command, COLOR_WHITE);
    EnqueueCommandQueue(&editor->Commands, command);

    static StringView status = AsStringView(" LIE - Lightweight Integrated Editor");
    MakePrintCommand(&command, status);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeClearLineCommand(&command, CLEAR_LINE_TO_END);
    EnqueueCommandQueue(&editor->Commands, command);

    u16 positionX = editor->FixedCursorX + editor->OffsetX;
    u16 positionY = editor->FixedCursorY + editor->OffsetY;
    u16 targetX = editor->Width - (u16)(Log10(positionY) + Log10(positionX) + 10);
    MakeMoveCursorCommand(&command, targetX, editor->Height);
    EnqueueCommandQueue(&editor->Commands, command);

    editor->Status.Length = 0;
    switch (editor->Mode)
    {
        case EDITOR_MODE_VIEW:
            AppendStringView(&editor->Status, AsStringView("- VIEW - "));
            break;
        case EDITOR_MODE_EDIT:
            AppendStringView(&editor->Status, AsStringView("- EDIT - "));
            break;
    }
    AppendUInt(&editor->Status, positionY);
    AppendStringView(&editor->Status, AsStringView(":"));
    AppendUInt(&editor->Status, positionX);
    MakePrintCommand(&command, ToStringView(&editor->Status));
    EnqueueCommandQueue(&editor->Commands, command);

    MakeSetForegroundCommand(&command, COLOR_RESET);
    EnqueueCommandQueue(&editor->Commands, command);

    MakeSetBackgroundCommand(&command, COLOR_RESET);
    EnqueueCommandQueue(&editor->Commands, command);
}

void RefreshScreen(Editor* editor)
{
    Command command;

    MakeHideCursorCommand(&command);
    EnqueueCommandQueue(&editor->Commands, command);

    PrintLines(editor);

    if (editor->StatusTimeout == 0)
    {
        PrintEditorInfo(editor);
    }
    else
    {
        PrintStatusMessage(editor);
        editor->StatusTimeout -= 1;
    }

    MakeMoveCursorCommand(&command, editor->FixedCursorX, editor->FixedCursorY);
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
    usize rowIndex = editor->CursorY - 1 + editor->OffsetY;
    usize rowLength = editor->Rows.Values[rowIndex].Length;

    editor->OffsetX = (u16)(rowLength - Min(rowLength, editor->Width - 1));
    editor->CursorX = (u16)(rowLength + 1 - editor->OffsetX);
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
    u16 remaningRows = (u16)(editor->Rows.Count - editor->OffsetY);

    u16 move = Min(remaningRows - editor->CursorY, count);
    move = Min(move, editor->Height - editor->CursorY - 1);
    editor->CursorY += move;

    u16 offset = Min(remaningRows - Min(remaningRows, editor->Height), count - move);
    editor->OffsetY += offset;
}

void MoveLeft(Editor* editor, u16 count)
{
    usize rowIndex = editor->CursorY - 1 + editor->OffsetY;

    u16 move = Min(editor->FixedCursorX - 1, count);
    editor->CursorX = editor->FixedCursorX - move;

    u16 offset = Min(editor->OffsetX, count - move);
    editor->OffsetX -= offset;

    u16 excess = count - move - offset;
    if (excess > 0 && rowIndex > 0)
    {
        MoveUp(editor, 1);
        MoveCursorToLineEnd(editor);
        if (excess > 1)
        {
            MoveLeft(editor, excess - 1);
        }
    }
}

void MoveRight(Editor* editor, u16 count)
{
    usize rowIndex = editor->CursorY - 1 + editor->OffsetY;
    usize rowLength = editor->Rows.Values[rowIndex].Length;

    u16 remaining = (u16)(rowLength + 1 - editor->OffsetX);

    u16 move = Min(remaining - editor->FixedCursorX, count);
    move = Min(move, editor->Width - editor->FixedCursorX);
    editor->CursorX = editor->FixedCursorX + move;

    u16 offset = Min(remaining - editor->CursorX, count - move);
    editor->OffsetX += offset;

    u16 excess = count - move - offset;
    if (excess > 0 && rowIndex < editor->Rows.Count - 1)
    {
        MoveDown(editor, 1);
        MoveCursorToLineStart(editor);
        if (excess > 1)
        {
            MoveRight(editor, excess - 1);
        }
    }
}

void InsertCharacter(Editor* editor, char character)
{
    usize rowIndex = editor->FixedCursorY - 1 + editor->OffsetY;
    String* currentRow = &editor->Rows.Values[rowIndex];
    usize insertIndex = editor->FixedCursorX + editor->OffsetX - 1;
    InsertChar(currentRow, insertIndex, character);
    MoveRight(editor, 1);
}

void InsertTab(Editor* editor)
{
    usize rowIndex = editor->FixedCursorY - 1 + editor->OffsetY;
    String* currentRow = &editor->Rows.Values[rowIndex];
    usize insertIndex = editor->FixedCursorX + editor->OffsetX - 1;

    u16 tabSize = 4 - (insertIndex % 4);
    for (u16 index = 0; index < tabSize; index += 1)
    {
        InsertChar(currentRow, insertIndex + index, ' ');
    }

    MoveRight(editor, tabSize);
}

void InsertNewLine(Editor* editor)
{
    InsertToRows(&editor->Rows, EmptyString, editor->CursorY + editor->OffsetY);

    usize rowIndex = editor->FixedCursorY - 1 + editor->OffsetY;
    String* currentRow = &editor->Rows.Values[rowIndex];

    u16 insertIndex = editor->FixedCursorX - 1 + editor->OffsetX;
    StringView contentToEnd = MakeStringView(currentRow, insertIndex, currentRow->Length);
    if (contentToEnd.Length > 0)
    {
        String* nextRow = &editor->Rows.Values[rowIndex + 1];
        AppendStringView(nextRow, contentToEnd);
        EraseString(currentRow, insertIndex, currentRow->Length);
    }

    MoveCursorToLineStart(editor);
    MoveDown(editor, 1);
}

void DeleteCharacter(Editor* editor)
{
    usize rowIndex = editor->FixedCursorY - 1 + editor->OffsetY;
    String* currentRow = &editor->Rows.Values[rowIndex];

    u16 deleteIndex = editor->FixedCursorX - 1 + editor->OffsetX;
    if (deleteIndex > 0)
    {
        MoveLeft(editor, 1);
        EraseString(currentRow, deleteIndex - 1, deleteIndex);
    }
    else if (rowIndex > 0)
    {
        MoveUp(editor, 1);
        MoveCursorToLineEnd(editor);

        String* previousRow = &editor->Rows.Values[rowIndex - 1];
        StringView contentToEnd = MakeStringView(currentRow, deleteIndex, currentRow->Length);
        if (contentToEnd.Length > 0)
        {
            AppendStringView(previousRow, contentToEnd);
        }

        RemoveFromRows(&editor->Rows, rowIndex);
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
                    if (event->Key.Value == 'Q' && event->Key.Modifiers == KEY_MODIFIER_CONTROL)
                    {
                        editor->Running = false;
                    }
                    else if (event->Key.Value == 'W' && event->Key.Modifiers == KEY_MODIFIER_CONTROL)
                    {
                        editor->Mode = EDITOR_MODE_VIEW;
                    }
                    else if (event->Key.Value == 'E' && event->Key.Modifiers == KEY_MODIFIER_CONTROL)
                    {
                        editor->Mode = EDITOR_MODE_EDIT;
                    }
                    else if (event->Key.Value == 'S' && event->Key.Modifiers == KEY_MODIFIER_CONTROL)
                    {
                        SaveFile(editor);
                    }
                    else if (editor->Mode == EDITOR_MODE_EDIT)
                    {
                        InsertCharacter(editor, event->Key.Value);
                    }
                    break;

                case KEY_CODE_TAB:
                    if (editor->Mode == EDITOR_MODE_EDIT)
                    {
                        InsertTab(editor);
                    }
                    break;

                case KEY_CODE_ENTER:
                    if (editor->Mode == EDITOR_MODE_EDIT)
                    {
                        InsertNewLine(editor);
                    }
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

                case KEY_CODE_BACKSPACE:
                    if (editor->Mode == EDITOR_MODE_EDIT)
                    {
                        DeleteCharacter(editor);
                    }
                    break;

                default:
                    break;
            }
            break;
    }
}

bool EditorPrompt(Editor* editor, String* prompt, StringView* out)
{
    usize initialLength = prompt->Length;

    Event event;
    while (true)
    {
        PrepareStatusMessage(editor, ToStringView(prompt), false);
        RefreshScreen(editor);

        if (ReadEvent(editor->Terminal, &event))
        {
            if (event.Kind != EVENT_KEY)
                continue;

            if (event.Key.Code == KEY_CODE_ENTER)
            {
                editor->StatusTimeout = 0;
                *out = MakeStringView(prompt, initialLength, prompt->Length);
                return true;
            }

            if (event.Key.Code == KEY_CODE_ESCAPE)
            {
                editor->StatusTimeout = 0;
                return false;
            }

            if (event.Key.Code == KEY_CODE_BACKSPACE && prompt->Length > initialLength)
            {
                prompt->Length -= 1;
                continue;
            }

            if (event.Key.Code == KEY_CODE_CHARACTER)
            {
                AppendChar(prompt, event.Key.Value);
            }
        }
    }
}
