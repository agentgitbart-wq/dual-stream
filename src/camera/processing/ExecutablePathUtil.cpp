#include "ExecutablePathUtil.h"
#include <windows.h>

std::filesystem::path GetExecutableDirectory() {
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (length == 0) {
        return {};
    }
    return std::filesystem::path(buffer).parent_path();
}
