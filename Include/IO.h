#ifndef __LIE_IO_H__
#define __LIE_IO_H__

#include <Core.h>
#include <Utility.h>

bool IsTTY();

bool ReadStdIn(void* destination, usize size);
bool WriteStdOut(const void* source, usize size);

bool ReadFile(StringView filepath, String* destination);
bool WriteFile(StringView filepath, StringView source);
bool AppendFile(StringView filepath, StringView source);

#endif