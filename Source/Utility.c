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
    u8* dst = (u8*)destination;

    while (size >= sizeof(u64))
    {
        *(u64*)dst = *(u64*)&value;
        dst += sizeof(u64);
        size -= sizeof(u64);
    }

    while (size >= sizeof(u8))
    {
        *dst = value;
        dst += sizeof(u8);
        size -= sizeof(u8);
    }
}

void MemoryCopy(void* destination, void* source, usize size)
{
    u8* dst = (u8*)destination;
    u8* src = (u8*)source;

    while (size >= sizeof(u64))
    {
        *(u64*)dst = *(u64*)src;
        dst += sizeof(u64);
        src += sizeof(u64);
        size -= sizeof(u64);
    }

    while (size >= sizeof(u8))
    {
        *dst = *src;
        dst += sizeof(u8);
        src += sizeof(u8);
        size -= sizeof(u8);
    }
}
