#ifndef __LIE_EVENT_H__
#define __LIE_EVENT_H__

#include <Core.h>

typedef enum EventKind
{
    EK_None = 0,
    EK_Key = 1,
} EventKind;

typedef struct KeyEvent
{
    u8 Code;
} KeyEvent;

typedef struct Event
{
    EventKind Kind;
    union
    {
        KeyEvent Key;
    };
} Event;

#endif