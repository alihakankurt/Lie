#ifndef __LIE_UTILITY_H__
#define __LIE_UTILITY_H__

#include <Core.h>

bool IsDigit(u8 c);
bool IsUppercase(u8 c);
bool IsLowercase(u8 c);

void* MemoryAllocate(usize size);
void MemoryFree(void* source);

void MemoryClear(void* destination, usize size);
void MemorySet(void* destination, u8 value, usize size);
void MemoryCopy(void* destination, void* source, usize size);

#endif