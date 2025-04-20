#include "injector.hpp"

bool CManualMap(DWORD targetProcessID, const char* dllPath) {
    
    // Open the target process
    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, targetProcessID);
    if (hProcess == NULL) {
        return FALSE; // Failed to open target process
    }

    // Read the image headers of the DLL to be injected
    HANDLE hFile = CreateFileA(dllPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hProcess);
        return false; // Failed to open DLL file
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    LPVOID fileBuffer = VirtualAlloc(NULL, fileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (fileBuffer == NULL) {
        CloseHandle(hFile);
        CloseHandle(hProcess);
        return false; // Failed to allocate memory for DLL file
    }

    DWORD bytesRead;
    ReadFile(hFile, fileBuffer, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    // Allocate memory in the target process for the DLL path
    LPVOID remotePath = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remotePath == NULL) {
        VirtualFree(fileBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false; // Failed to allocate memory in the target process
    }

    // Write the DLL path into the target process
    WriteProcessMemory(hProcess, remotePath, dllPath, strlen(dllPath) + 1, NULL);

    // Get the address of LoadLibraryA function in kernel32.dll
    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (loadLibraryAddr == NULL) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        VirtualFree(fileBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false; // LoadLibraryA not found
    }

    // Create a remote thread in the target process to execute LoadLibraryA with the DLL path
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remotePath, 0, NULL);
    if (hThread == NULL) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        VirtualFree(fileBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false; // Failed to create remote thread
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
    VirtualFree(fileBuffer, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return true;
}