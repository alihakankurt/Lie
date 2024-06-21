#include <IO.h>

#if defined(LIE_PLATFORM_LINUX) || defined(LIE_PLATFORM_MACOS)

#include <unistd.h>

bool IsTTY()
{
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
}

bool ReadStdIn(void* destination, usize size)
{
    isize readBytes = read(STDIN_FILENO, destination, size);
    return readBytes >= 0 && (usize)readBytes == size;
}

bool WriteStdOut(const void* source, usize size)
{
    isize writtenBytes = write(STDOUT_FILENO, source, size);
    return writtenBytes >= 0 && (usize)writtenBytes == size;
}

#endif