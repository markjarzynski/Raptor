#pragma once

#include "Allocator.h"
#include "Types.h"

namespace Raptor
{
namespace Core
{

static const uint32 MAX_FILENAME_LENGTH = 512;

using FileHandle = FILE*;

void DirectoryFromPath(char* path);
void FilenameFromPath(char* path);

void CurrentDirectory(char* path);
void ChangeDirectory(const char* path);

struct FileReadResult
{
    char* data;
    sizet size;
}; // struct FileReadResult

static long FileGetSize(FileHandle file);
FileReadResult FileReadBinary(const char* filename, Allocator* allocator);

} // namespace Core
} // namespace Raptor