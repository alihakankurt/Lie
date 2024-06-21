#ifndef __LIE_IO_H__
#define __LIE_IO_H__

#include <Core.h>

typedef struct File File;

bool IsTTY();

bool ReadStdIn(void* destination, usize size);
bool WriteStdOut(const void* source, usize size);

#endif