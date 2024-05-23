#ifndef __LIE_EVENT_H__
#define __LIE_EVENT_H__

#include <Core.h>

typedef enum EventKind
{
    EVENT_NONE = 0,
    EVENT_KEY = 1,
} EventKind;

typedef enum KeyCode
{
    KEY_CODE_NONE = 0,
    KEY_CODE_CHARACTER = 1,
    KEY_CODE_FUNCTION = 2,
    KEY_CODE_TAB = 9,
    KEY_CODE_ENTER = 10,
    KEY_CODE_ESCAPE = 27,
    KEY_CODE_INSERT = 50,
    KEY_CODE_DELETE = 51,
    KEY_CODE_PAGE_UP = 53,
    KEY_CODE_PAGE_DOWN = 54,
    KEY_CODE_UP = 65,
    KEY_CODE_DOWN = 66,
    KEY_CODE_RIGHT = 67,
    KEY_CODE_LEFT = 68,
    KEY_CODE_END = 70,
    KEY_CODE_HOME = 72,
    KEY_CODE_BACKSPACE = 127,
} KeyCode;

typedef enum KeyModifier
{
    KEY_MODIFIER_NONE = 0,
    KEY_MODIFIER_SHIFT = 1,
    KEY_MODIFIER_ALT = 2,
    KEY_MODIFIER_CONTROL = 4,
} KeyModifier;

typedef struct Event
{
    EventKind Kind;
    union
    {
        struct KeyEventData
        {
            KeyCode Code;
            KeyModifier Modifiers;
            u16 Value;
        } Key;
    };
} Event;

void MakeKeyEvent(Event* event, KeyCode code, KeyModifier modifiers, u16 value);

#endif