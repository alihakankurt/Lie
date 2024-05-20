#ifndef __LIE_TERMINAL_H__
#define __LIE_TERMINAL_H__

#include <Core.h>
#include <Event.h>
#include <Command.h>

void EnableRawMode();
void DisableRawMode();

bool PollEvent(Event* event);

void ProcessCommandQueue(CommandQueue* queue);

#endif