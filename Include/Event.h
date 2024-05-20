#ifndef __LIE_EVENT_H__
#define __LIE_EVENT_H__

#include <Core.h>

typedef enum EventKind
{
    KeyEvent = 1,
} EventKind;

typedef enum KeyCode
{
    Character = 1,
    FunctionKey = 2,
    TabKey = 9,
    EnterKey = 10,
    EscapeKey = 27,
    InsertKey = 50,
    DeleteKey = 51,
    PageUpKey = 53,
    PageDownKey = 54,
    UpKey = 65,
    DownKey = 66,
    RightKey = 67,
    LeftKey = 68,
    EndKey = 70,
    HomeKey = 72,
    BackspaceKey = 127,
} KeyCode;

typedef enum KeyModifier
{
    ShiftModifier = 1,
    AltModifier = 2,
    CtrlModifier = 4,
} KeyModifier;

typedef struct Event
{
    EventKind Kind;
    union
    {
        struct KeyEventData
        {
            KeyCode Code;
            u8 Value;
            KeyModifier Modifiers;
        } Key;
    };
} Event;

#endif