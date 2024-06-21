#include <Utility.h>

#if defined(LIE_PLATFORM_LINUX) || defined(LIE_PLATFORM_MACOS)

#include <sys/mman.h>

void* MemoryAllocate(usize size)
{
    return mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void MemoryFree(void* source)
{
    munmap(source, 0);
}

#endif