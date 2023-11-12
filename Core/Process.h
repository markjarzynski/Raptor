#pragma once

namespace Raptor
{
namespace Core
{

bool ProcessExecute(const char* cwd, const char* path, const char* args, const char* search_error_string = "");
const char* ProcessGetOutput();

}; // namespace Core
}; // namespace Raptor