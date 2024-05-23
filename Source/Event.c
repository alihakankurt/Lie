#include <Event.h>

void MakeKeyEvent(Event* event, KeyCode code, KeyModifier modifiers, u16 value)
{
    event->Kind = EVENT_KEY;
    event->Key.Code = code;
    event->Key.Modifiers = modifiers;
    event->Key.Value = value;
}
