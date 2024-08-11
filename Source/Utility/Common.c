#include <Utility.h>

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

void MemoryClear(void* destination, usize size)
{
    MemorySet(destination, 0, size);
}

void MemorySet(void* destination, u8 value, usize size)
{
    u64 value64 = value;
    value64 |= value64 << 8;
    value64 |= value64 << 16;
    value64 |= value64 << 32;

    while (size >= 8)
    {
        *(u64*)destination = value64;
        destination = (u64*)destination + 1;
        size -= 8;
    }

    while (size >= 1)
    {
        *(u8*)destination = value;
        destination = (u8*)destination + 1;
        size -= 1;
    }
}

void MemoryCopy(void* destination, const void* source, usize size)
{
    if (destination == source || size == 0)
        return;

    if (destination < source)
    {
        while (size >= 8)
        {
            *(u64*)destination = *(u64*)source;
            destination = (u64*)destination + 1;
            source = (u64*)source + 1;
            size -= 8;
        }

        while (size >= 1)
        {
            *(u8*)destination = *(u8*)source;
            destination = (u8*)destination + 1;
            source = (u8*)source + 1;
            size -= 1;
        }
    }
    else
    {
        destination = (u8*)destination + size;
        source = (u8*)source + size;

        while (size >= 8)
        {
            destination = (u64*)destination - 1;
            source = (u64*)source - 1;
            *(u64*)destination = *(u64*)source;
            size -= 8;
        }

        while (size >= 1)
        {
            destination = (u8*)destination - 1;
            source = (u8*)source - 1;
            *(u8*)destination = *(u8*)source;
            size -= 1;
        }
    }
}

usize GetStrLength(const char* str)
{
    usize length = 0;
    while (str[length] != '\0')
        length++;

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

void ExtendString(String* string, usize capacity)
{
    if (string->Capacity >= capacity)
        return;

    u8* content = MemoryAllocate(capacity + 1);
    if (string->Content != NULL)
    {
        MemoryCopy(content, string->Content, string->Length);
        MemoryFree(string->Content);
    }

    string->Content = content;
    string->Capacity = capacity;
}

void EraseString(String* string, usize start, usize end)
{
    if (start >= end)
        return;

    MemoryCopy(string->Content + start, string->Content + end, string->Length - end);
    string->Length -= end - start;
    string->Content[string->Length] = '\0';
}

void AppendChar(String* string, char c)
{
    ExtendString(string, string->Length + 1);

    string->Content[string->Length] = (u8)c;
    string->Length += 1;

    string->Content[string->Length] = '\0';
}

void AppendStr(String* string, const char* str)
{
    usize strLength = GetStrLength(str);
    ExtendString(string, string->Length + strLength);

    MemoryCopy(string->Content + string->Length, str, strLength);
    string->Length += strLength;

    string->Content[string->Length] = '\0';
}

void AppendString(String* string, String* other)
{
    ExtendString(string, string->Length + other->Length);

    MemoryCopy(string->Content + string->Length, other->Content, other->Length);
    string->Length += other->Length;

    string->Content[string->Length] = '\0';
}

void AppendStringView(String* string, StringView view)
{
    ExtendString(string, string->Length + view.Length);

    MemoryCopy(string->Content + string->Length, view.Content, view.Length);
    string->Length += view.Length;

    string->Content[string->Length] = '\0';
}

void AppendUInt(String* string, u64 value)
{
    static u8 buffer[32];
    usize index = 31;
    while (value > 0)
    {
        buffer[index] = (u8)('0' + (value % 10));
        value /= 10;
        index -= 1;
    }

    StringView view = {.Length = 32 - index - 1, .Content = buffer + index + 1};
    AppendStringView(string, view);
}

void InsertChar(String* string, usize index, char c)
{
    if (index > string->Length)
    {
        AppendChar(string, c);
        return;
    }

    ExtendString(string, string->Length + 1);

    MemoryCopy(string->Content + index + 1, string->Content + index, string->Length - index);
    string->Content[index] = (u8)c;
    string->Length += 1;

    string->Content[string->Length] = '\0';
}

StringView MakeStringView(String* string, usize start, usize end)
{
    StringView view;
    view.Length = end - start;
    view.Content = string->Content + start;
    return view;
}

bool TryParseUInt(StringView view, u64* value)
{
    *value = 0;
    for (usize index = 0; index < view.Length; index++)
    {
        if (!IsDigit(view.Content[index]))
            return false;

        *value = *value * 10 + (view.Content[index] - '0');
    }

    return true;
}

u64 Log10(u64 value)
{
    // clang-format off
    static const u64 Log10Table[] = {
        0, 0, 0, 0, 1, 1, 1, 2,
        2, 2, 3, 3, 3, 3, 4, 4,
        4, 5, 5, 5, 6, 6, 6, 6,
        7, 7, 7, 8, 8, 8, 9, 9,
        9, 9, 10, 10, 10, 11, 11, 11,
        12, 12, 12, 12, 13, 13, 13, 14,
        14, 14, 15, 15, 15, 15, 16, 16,
        16, 17, 17, 17, 18, 18, 18, 18, 19,
    };
    static const u64 TenthPowers[] = {
        1, 10, 100, 1000, 10000, 100000, 1000000,
        10000000, 100000000, 1000000000, 10000000000,
        100000000000, 1000000000000, 10000000000000,
        100000000000000, 1000000000000000, 10000000000000000,
        100000000000000000, 1000000000000000000,
    };
    // clang-format on

    u64 digits = Log10Table[value ? 64 - __builtin_clzll(value) : 0];
    return digits + (value >= TenthPowers[digits]);
}
