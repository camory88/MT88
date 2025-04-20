#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <filesystem>

#include "menu.hpp"
#include "injector.hpp"


struct menuMenuGlobals
{
    std::string selectedFile = "";
    int processId = 0;
}MenuGlobals;


void ShowInjectorMenu(){
    
    static std::string status;

    ImGui::Begin("Injector Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Input field for process ID
    ImGui::InputInt("Process ID", &MenuGlobals.processId);
    // Button to open file selector
    
    if (ImGui::Button("Select DLL")) {
        char filePath[MAX_PATH] = "";
        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFilter = "DLL Files\0*.dll\0All Files\0*.*\0";
        ofn.lpstrFile = filePath;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn)) {
            MenuGlobals.selectedFile = filePath;
        }
    }

    // Display selected file
    if (!MenuGlobals.selectedFile.empty()) {
        ImGui::Text("Selected File: %s", MenuGlobals.selectedFile.c_str());
    }

    // Inject button
    if (ImGui::Button("Inject")) {
        if (MenuGlobals.processId > 0) {
            // Perform injection logic here
            bool injectionSuccess = CManualMap(MenuGlobals.processId, MenuGlobals.selectedFile.c_str()); // Replace with your actual injection function
            if (injectionSuccess) {
                status = "Injection successful!";
            } else {
                status = "Injection failed!";
            }
        } else {
            status = "Invalid Process ID!";
        }
    }

    // Display status message
    if (!status.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", status.c_str());
    }

    ImGui::End();
}

void ShowQickInjectMenu();

bool EnsureDirectoryExists(const std::string& path) {
    // Convert forward slashes to backslashes for Windows
    std::string normalizedPath = path;
    for (char& c : normalizedPath) {
        if (c == '/') c = '\\';
    }

    // Check if path exists
    DWORD fileAttr = GetFileAttributesA(normalizedPath.c_str());
    if (fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        return true; // Directory already exists
    }

    // Find the parent directory by locating the last backslash
    size_t lastSlash = normalizedPath.find_last_of('\\');
    if (lastSlash != std::string::npos && lastSlash > 0) {
        // Recursively ensure parent directory exists
        std::string parentPath = normalizedPath.substr(0, lastSlash);
        if (!EnsureDirectoryExists(parentPath)) {
            return false;
        }
    }

    // Create the directory
    BOOL result = CreateDirectoryA(normalizedPath.c_str(), NULL);
    if (result || GetLastError() == ERROR_ALREADY_EXISTS) {
        return true;
    }

    return false;
}

// Improved Quick DLL Selector Menu
void ShowQuickDllSelectMenu() {
    static std::string selectedFile = "None";
    static std::string statusMessage = "Ready";
    static std::vector<std::string> dllFiles;
    static bool needsRefresh = true; // Flag to trigger initial load

    // Get directory path and ensure it exists
    static std::string dllPath;
    if (dllPath.empty()) {
        std::string publicDocs = GetPublicDocumentsPath();
        if (publicDocs.empty()) {
            statusMessage = "Error: Failed to retrieve Public Documents path";
            ImGui::Begin("Quick DLL Selector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", statusMessage.c_str());
            ImGui::End();
            return;
        }
        dllPath = publicDocs + "\\SwCheats\\DLLs\\";
        if (!EnsureDirectoryExists(dllPath)) {
            statusMessage = "Error: Failed to create directory: " + dllPath;
            ImGui::Begin("Quick DLL Selector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", statusMessage.c_str());
            ImGui::End();
            return;
        }
    }

    // Begin ImGui window
    ImGui::Begin("Quick DLL Selector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Refresh button
    if (ImGui::Button("Refresh")) {
        needsRefresh = true;
        statusMessage = "Refreshing...";
    }

    // Load DLL files if needed
    if (needsRefresh) {
        dllFiles = GetDllFiles(dllPath);
        needsRefresh = false;
        if (dllFiles.empty()) {
            statusMessage = "No .dll files found in " + dllPath;
        } else {
            statusMessage = "Found " + std::to_string(dllFiles.size()) + " .dll file(s)";
        }
    }

    // Display directory and status
    ImGui::Text("Directory: %s", dllPath.c_str());
    ImGui::Text("Selected DLL: %s", selectedFile.c_str());
    ImGui::Text("Status: %s", statusMessage.c_str());
    ImGui::Separator();

    // Scrollable list of DLLs
    ImGui::BeginChild("DLL List", ImVec2(0, 200), true);
    if (dllFiles.empty()) {
        ImGui::Text("No .dll files available.");
    } else {
        for (size_t i = 0; i < dllFiles.size(); ++i) {
            const auto& dllFile = dllFiles[i];
            ImGui::Text("%s", dllFile.c_str());
            ImGui::SameLine();
            std::string buttonLabel = "Select##" + std::to_string(i);
            if (ImGui::Button(buttonLabel.c_str())) {
                MenuGlobals.selectedFile = dllPath+dllFile;
                statusMessage = "Selected: " + dllFile;
            }
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void ShowProcessFilterMenu() {
    static char filterBuffer[128] = "";
    static std::vector<ProcessInfo> processes;
    static std::string status;
    static bool needsRefresh = true;

    ImGui::Begin("Process Filter Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Input field for filtering processes
    bool filterChanged = ImGui::InputText("Filter by Name", filterBuffer, IM_ARRAYSIZE(filterBuffer),
        ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    bool refreshClicked = ImGui::Button("Refresh");

    // Refresh process list when needed
    if (filterChanged || refreshClicked || needsRefresh) {
        auto [success, result] = GetProcessesByName(filterBuffer);
        if (success) {
            processes = std::move(result);
            status.clear();
        } else {
            processes.clear();
            status = "Error: Failed to retrieve process list";
        }
        needsRefresh = false;
    }

    // Display process list in a table
    ImGui::Separator();
    ImGui::Text("Processes: %zu found", processes.size());

    if (ImGui::BeginTable("ProcessTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (const auto& process : processes) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", process.name.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%u", process.pid);
            ImGui::TableSetColumnIndex(2);
            if(ImGui::Button("Select", ImVec2(0, 0))) // Button to inject DLL into the process
            {
                MenuGlobals.processId = process.pid;
                status = "Selected: " + process.name;
            }
        }
        ImGui::EndTable();
    }

    // Display error message if any
    if (!status.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", status.c_str());
    }

    ImGui::End();
}

inline std::string to_lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return str;
}

std::pair<bool, std::vector<ProcessInfo>> GetProcessesByName(const std::string& filterName) {
    std::vector<ProcessInfo> filteredProcesses;
    std::vector<DWORD> processIds(1024);
    DWORD bytesReturned;

    // Get list of process IDs
    if (!EnumProcesses(processIds.data(), processIds.size() * sizeof(DWORD), &bytesReturned)) {
        return { false, filteredProcesses };
    }

    std::string lowerFilter = to_lower(filterName);
    size_t processCount = bytesReturned / sizeof(DWORD);

    // Iterate through process IDs
    for (size_t i = 0; i < processCount; ++i) {
        if (DWORD pid = processIds[i]; pid != 0) {
            HandleGuard hProcess(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid));
            if (hProcess.get() != nullptr) {
                char processName[MAX_PATH] = "";
                if (GetModuleBaseNameA(hProcess.get(), nullptr, processName, MAX_PATH) > 0) {
                    std::string name = processName;
                    if (filterName.empty() || to_lower(name).find(lowerFilter) != std::string::npos) {
                        filteredProcesses.push_back({ name, pid });
                    }
                }
            }
        }
    }

    // Sort processes alphabetically
    std::sort(filteredProcesses.begin(), filteredProcesses.end(),
        [](const ProcessInfo& a, const ProcessInfo& b) {
            return to_lower(a.name) < to_lower(b.name);
        });

    return { true, filteredProcesses };
}

// Function to get all .dll files in a directory
std::vector<std::string> GetDllFiles(const std::string& directory) {
    std::vector<std::string> dllFiles;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            dllFiles.push_back(entry.path().filename().string());
        }
    }
    return dllFiles;
}

std::string GetPublicDocumentsPath(){
    CHAR path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, path))) {
        return std::string(path);
    }
    return ""; // Fallback to empty string if retrieval fails
}

