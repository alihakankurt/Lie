#include <Utility.h>

#include <stdlib.h>

bool IsDigit(u8 c)
{
    return '0' <= c && c <= '9';
}

bool IsUppercase(u8 c)
{
    return 'A' <= c && c <= 'Z';
}

bool IsLowercase(u8 c)
{
    return 'a' <= c && c <= 'z';
}

void* MemoryAllocate(usize size)
{
    return malloc(size);
}

void MemoryFree(void* source)
{
    free(source);
}

void MemoryClear(void* destination, usize size)
{
    MemorySet(destination, 0, size);
}

void MemorySet(void* destination, u8 value, usize size)
{
    while (size >= sizeof(u8))
    {
        *(u8*)destination = value;
        destination = (u8*)destination + 1;
        size -= sizeof(u8);
    }
}

void MemoryCopy(void* destination, const void* source, usize size)
{
    while (size >= sizeof(u64))
    {
        *(u64*)destination = *(u64*)source;
        destination = (u64*)destination + 1;
        source = (u64*)source + 1;
        size -= sizeof(u64);
    }

    while (size >= sizeof(u8))
    {
        *(u8*)destination = *(u8*)source;
        destination = (u8*)destination + 1;
        source = (u8*)source + 1;
        size -= sizeof(u8);
    }
}

usize GetStrLength(const char* str)
{
    usize length = 0;
    while (str[length] != '\0')
    {
        length++;
    }

    return length;
}

void InitializeString(String* string)
{
    string->Length = 0;
    string->Capacity = 0;
    string->Content = NULL;
}

void FinalizeString(String* string)
{
    MemoryFree(string->Content);
}

void ExtendString(String* string, usize requiredCapactiy)
{
    if (string->Capacity >= requiredCapactiy)
        return;

    u8* newContent = MemoryAllocate(requiredCapactiy);
    if (string->Content != NULL)
    {
        MemoryCopy(newContent, string->Content, string->Length);
        MemoryFree(string->Content);
    }

    string->Content = newContent;
    string->Capacity = requiredCapactiy;
}

void AppendChar(String* string, char c)
{
    ExtendString(string, string->Length + 1);
    string->Content[string->Length] = (u8)c;
    string->Length += 1;
}

void AppendStr(String* string, const char* str)
{
    usize strLength = GetStrLength(str);
    ExtendString(string, string->Length + strLength);
    MemoryCopy(string->Content + string->Length, str, strLength);
    string->Length += strLength;
}

void AppendString(String* string, String* other)
{
    ExtendString(string, string->Length + other->Length);
    MemoryCopy(string->Content + string->Length, other->Content, other->Length);
    string->Length += other->Length;
}

void AppendStringView(String* string, StringView view)
{
    ExtendString(string, string->Length + view.Length);
    MemoryCopy(string->Content + string->Length, view.Content, view.Length);
    string->Length += view.Length;
}

StringView AsStringView(String* string)
{
    StringView view;
    view.Length = string->Length;
    view.Content = string->Content;
    return view;
}

StringView MakeStringView(String* string, usize start, usize end)
{
    StringView view;
    view.Length = end - start;
    view.Content = string->Content + start;
    return view;
}

u16 StringViewToUInt16(StringView view)
{
    u16 value = 0;
    for (usize index = 0; index < view.Length; index++)
    {
        if (!IsDigit(view.Content[index]))
            return 0;

        value = value * 10 + (view.Content[index] - '0');
    }

    return value;
}

StringView UInt16ToStringView(u16 value)
{
    static u8 buffer[16];
    StringView view;

    if (value == 0)
    {
        buffer[0] = '0';
        view.Length = 1;
        view.Content = buffer;
        return view;
    }

    usize index = 15;
    while (value > 0)
    {
        buffer[index] = (u8)('0' + (value % 10));
        value /= 10;
        index -= 1;
    }

    view.Length = 16 - index - 1;
    view.Content = buffer + index + 1;
    return view;
}
