#ifndef __LIE_UTILITY_H__
#define __LIE_UTILITY_H__

#include <Core.h>

#define Min(left, right) ((left) < (right) ? (left) : (right))
#define Max(left, right) ((left) > (right) ? (left) : (right))

void* MemoryAllocate(usize size);
void MemoryFree(void* source);

void MemoryClear(void* destination, usize size);
void MemorySet(void* destination, u8 value, usize size);
void MemoryCopy(void* destination, const void* source, usize size);


typedef struct String
{
    usize Length;
    usize Capacity;
    char* Content;
} String;

typedef struct StringView
{
    usize Length;
    const char* Content;
} StringView;

#define EmptyString     ((String){.Length = 0, .Capacity = 0, .Content = NULL})
#define EmptyStringView ((StringView){.Length = 0, .Content = NULL})

bool IsDigit(char c);
bool IsUppercase(char c);
bool IsLowercase(char c);
usize GetStrLength(const char* str);

void InitializeString(String* string);
void FinalizeString(String* string);
void ExtendString(String* string, usize capacity);
void EraseString(String* string, usize start, usize end);

void AppendChar(String* string, char c);
void AppendStr(String* string, const char* str);
void AppendString(String* string, String* other);
void AppendStringView(String* string, StringView view);
void AppendUInt(String* string, u64 value);

void InsertChar(String* string, usize index, char c);

#define AsStringView(str) ((StringView){.Length = sizeof(str) - 1, .Content = str})
StringView ToStringView(String* string);
StringView MakeStringView(String* string, usize start, usize end);

bool TryParseUInt(StringView view, u64* value);

u64 Log10(u64 value);

#endif