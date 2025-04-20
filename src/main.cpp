#include "main.hpp"
#include "menu.hpp"



// Renders ImGui interface
void RenderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

	
    ShowProcessFilterMenu();// process filter menu
	ShowInjectorMenu();// injector menu
	ShowQuickDllSelectMenu();// quick dll selector menu




    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Handle additional platform windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_context);
    }
}

int main() {
    // Initialize GLFW
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    // Configure OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(200, 200, "MT88 tool", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

	if(!MultiToolSetup()){ 	
		return EXIT_FAILURE;
    }
    // Setup ImGui
    SetupImGui(window);

    // Set clear color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Render ImGui
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        RenderImGui();

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup
    CleanupImGui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}