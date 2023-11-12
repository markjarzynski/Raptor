#if defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
// TODO
#endif

#include <stdio.h>

#include "Process.h"
#include "Types.h"
#include "Log.h"

namespace Raptor
{
namespace Core
{

static const uint32 PROCESS_LOG_BUFFER_SIZE = 256;
char s_process_log_buffer[PROCESS_LOG_BUFFER_SIZE];
static char k_process_output_buffer[1025];

#if defined(_WIN64)

void Win32GetError(char* buffer, uint32 size)
{
    DWORD errorCode = GetLastError();

    char* error_string;
    if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, errorCode, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPSTR)&error_string, 0, NULL ))
        return;

    sprintf_s(buffer, size, "%s", error_string);

    LocalFree(error_string);
}

bool ProcessExecute(const char* cwd, const char* path, const char* args, const char* search_error_string)
{
    HANDLE handle_stdin_pipe_read = NULL;
    HANDLE handle_stdin_pipe_write = NULL;
    HANDLE handle_stdout_pipe_read = NULL;
    HANDLE handle_std_pipe_write = NULL;

    SECURITY_ATTRIBUTES security_attributes = {sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE};

    BOOL ok = CreatePipe(&handle_stdin_pipe_read, &handle_stdin_pipe_write, &security_attributes, 0);
    if (ok == FALSE)
        return false;
    
    ok = CreatePipe( &handle_stdout_pipe_read, &handle_std_pipe_write, &security_attributes, 0 );
    if (ok == FALSE)
        return false;

    STARTUPINFOA startup_info {};
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    startup_info.hStdInput = handle_stdin_pipe_read;
    startup_info.hStdError = handle_std_pipe_write;
    startup_info.hStdOutput = handle_std_pipe_write;
    startup_info.wShowWindow = SW_SHOW;

    bool execution_success = false;
    PROCESS_INFORMATION process_info {};
    BOOL inherit_handles = TRUE;

    if (CreateProcessA(path, (char*)args, 0, 0, inherit_handles, 0, 0, cwd, &startup_info, &process_info))
    {
        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);
        execution_success = true;
    }
    else
    {
        Win32GetError(&s_process_log_buffer[0], PROCESS_LOG_BUFFER_SIZE);

        Raptor::Debug::Log("Execute process error: %s %s %s\n", path, args, cwd);
        Raptor::Debug::Log("Message: %s\n", s_process_log_buffer);
    }

    CloseHandle(handle_stdin_pipe_read);
    CloseHandle(handle_std_pipe_write);

    DWORD bytes_read;
    ok = ReadFile(handle_stdout_pipe_read, k_process_output_buffer, 1024, &bytes_read, nullptr);

    while (ok == TRUE)
    {
        k_process_output_buffer[bytes_read] = 0;
        Raptor::Debug::Log("%s", k_process_output_buffer);

        ok = ReadFile(handle_stdout_pipe_read, k_process_output_buffer, 1024, &bytes_read, nullptr);
    }

    if (strlen(search_error_string) > 0 && strstr(k_process_output_buffer, search_error_string))
    {
        execution_success = false;
    }

    Raptor::Debug::Log("\n");

    CloseHandle(handle_stdout_pipe_read);
    CloseHandle(handle_stdin_pipe_write);

    DWORD process_exit_code = 0;
    GetExitCodeProcess(process_info.hProcess, &process_exit_code);

    return execution_success;

}

const char* ProcessGetOutput()
{
    return k_process_output_buffer;
}

#else

// TODO

#endif

}; // namespace Core
}; // namespace Raptor