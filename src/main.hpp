#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <shlobj.h>

// Function to ensure a directory exists, creating it if necessary
bool EDE(const std::string& path) {
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
        if (!EDE(parentPath)) {
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

bool MultiToolSetup()
{
    CHAR path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, path))) {
        std::string publicDocs = std::string(path);

        std::string commenDLLs_path = publicDocs + "\\SwCheats\\DLLs\\";
        // maybe add other specific folders here?
        return EDE(commenDLLs_path);
    }

}


void SetupImGuiTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
}

// GLFW error callback
static void ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << "\n";
}

// Initializes ImGui context and backends
void SetupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.FontGlobalScale = 1.5f;

    // Setup transparent viewports
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.Colors[ImGuiCol_WindowBg].w = 0.0f;
        style.Colors[ImGuiCol_DockingEmptyBg].w = 0.0f;
    }

    SetupImGuiTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

// Cleans up ImGui resources
void CleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}