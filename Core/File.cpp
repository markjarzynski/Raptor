#include <string.h>
#if defined(_WIN64)
#include <windows.h>
#else
#include<unistd.h> 
#endif

#include <stdio.h>

#include "File.h"
#include "Debug.h"
#include "Types.h"

namespace Raptor
{
namespace Core
{

void DirectoryFromPath(char* path)
{
    char* last_point = strrchr(path, '.');
    char* last_separator = strrchr(path, '/');
    if (last_separator != nullptr && last_point > last_separator)
    {
        *(last_separator + 1) = 0;
    }
    else
    {
        last_separator = strrchr(path, '\\');
        if (last_separator != nullptr && last_point > last_separator)
        {
            *(last_separator + 1) = 0;
        }
        else
        {
            ASSERT_MESSAGE(false, "Error: Malformed path %s\n", path);
        }
    }
}

void FilenameFromPath(char* path)
{
    char* last_separator = strrchr(path, '/');
    if (last_separator == nullptr)
    {
        last_separator = strrchr(path, '\\');
    }

    if (last_separator != nullptr)
    {
        sizet length = strlen(last_separator + 1);
        memcpy(path, last_separator + 1, length);
        path[length] = 0;
    }
}

void CurrentDirectory(char* path)
{
#if defined(_WIN64)
    DWORD len = GetCurrentDirectoryA(MAX_FILENAME_LENGTH, path);
    path[len] = 0;
#else
    getcwd(path, MAX_FILENAME_LENGTH);
#endif
}

void ChangeDirectory(const char* path)
{
#if defined(_WIN64)
    if (!SetCurrentDirectoryA(path))
    {
        Raptor::Debug::Log("Error: Could not change directory to %s\n", path);
    }
#else
    if (chdir(path) != 0)
    {
        Raptor::Debug::Log("Error: Could not change directory to %s\n", path);
    }
#endif
}

static long FileGetSize(FileHandle file)
{
    long file_size;

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    return file_size;
}

FileReadResult FileReadBinary(const char* filename, Allocator* allocator)
{
    FileReadResult result {nullptr, 0};
    FileHandle file = fopen(filename, "rb");

    if (file)
    {
        sizet filesize = FileGetSize(file);
        result.data = (char*)allocator->allocate(filesize);
        fread(result.data, filesize, 1, file);
        result.size = filesize;
        fclose(file);
    }

    return result;
}

} // namespace Core
} // namespace Raptor