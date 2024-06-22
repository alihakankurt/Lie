#include <IO.h>

#if defined(LIE_PLATFORM_LINUX) || defined(LIE_PLATFORM_MACOS)

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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

bool ReadFile(StringView filepath, String* destination)
{
    i32 file = open(filepath.Content, O_RDONLY);
    if (file < 0)
        return false;

    struct stat fileStat;
    if (fstat(file, &fileStat) < 0)
    {
        close(file);
        return false;
    }

    usize fileSize = (usize)fileStat.st_size;
    ExtendString(destination, fileSize);

    usize readBytes = 0;
    while (readBytes < fileSize)
    {
        isize bytesRead = read(file, destination->Content + destination->Length, fileSize - destination->Length);
        if (bytesRead < 0)
        {
            close(file);
            return false;
        }

        readBytes += (usize)bytesRead;
    }

    destination->Length = readBytes;
    destination->Content[destination->Length] = '\0';
    close(file);
    return true;
}

bool WriteFile(StringView filepath, StringView source)
{
    i32 file = open(filepath.Content, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (file < 0)
        return false;

    usize writtenBytes = 0;
    while (writtenBytes < source.Length)
    {
        isize bytesWritten = write(file, source.Content + writtenBytes, source.Length - writtenBytes);
        if (bytesWritten < 0)
        {
            close(file);
            return false;
        }

        writtenBytes += (usize)bytesWritten;
    }

    close(file);
    return true;
}

bool AppendFile(StringView filepath, StringView source)
{
    i32 file = open(filepath.Content, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (file < 0)
        return false;

    usize writtenBytes = 0;
    while (writtenBytes < source.Length)
    {
        isize bytesWritten = write(file, source.Content + writtenBytes, source.Length - writtenBytes);
        if (bytesWritten < 0)
        {
            close(file);
            return false;
        }

        writtenBytes += (usize)bytesWritten;
    }

    close(file);
    return true;
}

#endif