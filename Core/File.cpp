#include <string.h>
#if defined(_WIN64)
#include <windows.h>
#else
#include<unistd.h> 
#endif

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

} // namespace Core
} // namespace Raptor