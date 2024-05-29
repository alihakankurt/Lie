#ifndef __LIE_EDITOR_H__
#define __LIE_EDITOR_H__

#include <Terminal.h>

typedef struct Editor
{
    Terminal* Terminal;
    CommandQueue Commands;

    u16 Width;
    u16 Height;

    u16 CursorX;
    u16 CursorY;
    bool Running;
} Editor;

void InitializeEditor(Editor* editor);
void FinalizeEditor(Editor* editor);

void RunWithEmptyFile(Editor* editor);

#endif