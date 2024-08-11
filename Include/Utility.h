#ifndef __LIE_UTILITY_H__
#define __LIE_UTILITY_H__

#include <Core.h>

#define Min(left, right) ((left) < (right) ? (left) : (right))
#define Max(left, right) ((left) > (right) ? (left) : (right))

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
    const u8* Content;
} StringView;

#define EmptyString ((String){.Length = 0, .Capacity = 0, .Content = NULL})

usize GetStrLength(const char* str);

void InitializeString(String* string);
void FinalizeString(String* string);
void ExtendString(String* string, usize capacity);

void AppendChar(String* string, char c);
void AppendStr(String* string, const char* str);
void AppendString(String* string, String* other);
void AppendStringView(String* string, StringView view);
void AppendUInt(String* string, u64 value);

void InsertChar(String* string, usize index, char c);

#define AsStringView(str) ((StringView){.Length = sizeof(str) - 1, .Content = (u8*)str})
StringView MakeStringView(String* string, usize start, usize end);

bool TryParseUInt(StringView view, u64* value);

u64 Log10(u64 value);

#endif