#ifndef __LIE_TERMINAL_H__
#define __LIE_TERMINAL_H__

#include <Core.h>
#include <Event.h>

void EnableRawMode();
void DisableRawMode();

bool PollEvent(Event* event);

#endif