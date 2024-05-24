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
void MemoryCopy(void* destination, const void* source, usize size);

typedef struct String
{
    usize Length;
    usize Capacity;
    u8* Content;
} String;

typedef struct StringView
{
    usize Length;
    u8* Content;
} StringView;

usize GetStrLength(const char* str);

void InitializeString(String* string);
void FinalizeString(String* string);

void AppendChar(String* string, char c);
void AppendStr(String* string, const char* str);
void AppendString(String* string, String* other);
void AppendStringView(String* string, StringView view);

#define AsStringView(str)            \
    (StringView)                     \
    {                                \
        .Length = GetStrLength(str), \
        .Content = (u8*)str          \
    }

StringView MakeStringView(String* string, usize start, usize end);

StringView UInt16ToStringView(u16 value);

#endif