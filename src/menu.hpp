#pragma once
#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <utility>
#include <memory>
#include <shlobj.h>
#include <TlHelp32.h>

// RAII wrapper for Windows HANDLE
class HandleGuard
{
public:
    explicit HandleGuard(HANDLE handle) : handle_(handle) {}
    ~HandleGuard()
    {
        if (handle_ != INVALID_HANDLE_VALUE)
            CloseHandle(handle_);
    }
    HandleGuard(const HandleGuard &) = delete;
    HandleGuard &operator=(const HandleGuard &) = delete;
    HANDLE get() const { return handle_; }

private:
    HANDLE handle_;
};
// Structure to hold process information
struct ProcessInfo
{
    std::string name;
    DWORD pid;
};

inline std::string to_lower(std::string str);                                                // Utility to convert string to lowercase for case-insensitive comparison
std::pair<bool, std::vector<ProcessInfo>> GetProcessesByName(const std::string &filterName); // Retrieves processes with optional name filter using EnumProcesses
std::vector<std::string> GetDllFiles(const std::string& directory);                      // Retrieves all .dll files in a specified directory
std::string GetPublicDocumentsPath(); // Function to get the Public Documents folder path




void ShowProcessFilterMenu();                                                                // Displays ImGui menu for process filtering and display   
void ShowInjectorMenu();                                                                     // Displays ImGui menu for injecting DLLs into selected processes
void ShowQickInjectMenu();                                                                   // Displays ImGui menu for quick injection of DLLs into selected processes
void ShowQuickDllSelectMenu();                                                                // Displays ImGui menu for selecting DLLs for quick injection


