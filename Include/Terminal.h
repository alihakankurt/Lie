#ifndef __LIE_TERMINAL_H__
#define __LIE_TERMINAL_H__

#include <Core.h>
#include <Event.h>
#include <Command.h>

typedef struct Terminal Terminal;

Terminal* CreateTerminal();
void DestroyTerminal(Terminal* terminal);

void EnableRawMode(Terminal* terminal);
void DisableRawMode(Terminal* terminal);

bool GetTerminalSize(Terminal* terminal, u16* width, u16* height);
bool GetCursorPosition(Terminal* terminal, u16* x, u16* y);

bool ReadEvent(Terminal* terminal, Event* event);

void ProcessCommandQueue(Terminal* terminal, CommandQueue* queue);

#endif