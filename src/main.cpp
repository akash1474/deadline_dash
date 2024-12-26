#include "GLFW/glfw3.h"
#include "Log.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "pch.h"
#include <chrono>
#include <ctime>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_img.h"
#include <dwmapi.h>
#include <windef.h>
#include <wingdi.h>
#include "./resources/AppIcon.embed"
#include "Utils.h"
#include "Application.h"
#include <dwrite.h>
#include <wrl.h>

#define IMGUI_DATEPICKER_YEAR_MIN 1970
#define IMGUI_DATEPICKER_YEAR_MAX 3000

int width{400};
int height{200};
int targetFPS = 60;

void draw(GLFWwindow* window)
{
    glfwGetWindowSize(window, &width, &height);
    // Rendering code goes here
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    #ifdef GL_USE_OPENGL_LATEST
        ImGui_ImplOpenGL3_NewFrame();
    #else
        ImGui_ImplOpenGL2_NewFrame();
    #endif
        ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    Application::Render();


    // End of render
    ImGui::Render();
    #ifdef GL_USE_OPENGL_LATEST
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    #else
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    #endif
        glfwSwapBuffers(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    draw(window);
}

void WindowFocusCallback(GLFWwindow* window, int focused) {
    if (focused) targetFPS = 60;
    else targetFPS = 8; 
}


int main(void){
    GLFWwindow* window;
    #ifdef GL_DEBUG
    OpenGL::Log::Init();
    #endif

    if (!glfwInit()) return -1;

#ifdef GL_USE_OPENGL_LATEST
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
#endif

    window = glfwCreateWindow(width, height, "Deadline Dash", NULL, NULL);
    glfwSetWindowSizeLimits(window, 280, 150, GLFW_DONT_CARE, GLFW_DONT_CARE);
    // glfwSetWindowOpacity(GLFWwindow *window, float opacity)
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    GL_INFO("OPENGL - {}",(const char*)glGetString(GL_VERSION));
    HWND WinHwnd=glfwGetWin32Window(window);
    BOOL USE_DARK_MODE = true;
    BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(WinHwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,&USE_DARK_MODE, sizeof(USE_DARK_MODE)));


    // Initialize ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    #ifdef GL_USE_OPENGL_LATEST
        if(!ImGui_ImplOpenGL3_Init()){
    #else
        if(!ImGui_ImplOpenGL2_Init()){
    #endif
        GL_CRITICAL("FAILED INIT IMGUI");
        return 1;
    }

    GL_INFO("Initializing Fonts");
    io.Fonts->Clear();
    io.IniFilename=nullptr;
    io.LogFilename=nullptr;

    Application::Init(window);
    Application::LoadFonts();

    // glfwSwapInterval(1);
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 2.0f;
    style.ItemSpacing.y=6.0f;
    style.ScrollbarRounding=2.0f;


    GLFWimage images[1];
    images[0].pixels = stbi_load_from_memory(AppIcon, IM_ARRAYSIZE(AppIcon), &images[0].width, &images[0].height, 0, 4); // rgba channels
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);


    ApplyDraculaColorScheme();

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowFocusCallback(window, WindowFocusCallback);

    while (!glfwWindowShouldClose(window)) {

        glfwGetWindowSize(window, &width, &height);
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    #ifdef GL_USE_OPENGL_LATEST
        ImGui_ImplOpenGL3_NewFrame();
    #else
        ImGui_ImplOpenGL2_NewFrame();
    #endif
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Render Other Stuff


        Application::Render();

        // Render Imgui Stuff
    #ifdef GL_DEBUG
        ImGui::ShowDemoWindow();
    #endif


        // End of render
        ImGui::Render();
    #ifdef GL_USE_OPENGL_LATEST
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    #else
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    #endif

        //Must be called after ImGui_ImplOpenGL3_RenderDrawData/ImGui_ImplOpenGL2_RenderDrawData
        Application::SwitchDisplayModeIfNeeded(width, height);
        Application::UpdateFontsIfNeeded();
        glfwSwapBuffers(window);
        glfwPollEvents();


        //Handling FPS
        static std::chrono::high_resolution_clock::time_point mLastFrameTime;
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedTime = currentTime - mLastFrameTime;

        double targetFrameTime = 1.0 / targetFPS;
        if (elapsedTime.count() < targetFrameTime){
            double remainingTime = targetFrameTime - elapsedTime.count();
            std::this_thread::sleep_for(std::chrono::duration<double>(remainingTime));
            continue;
        }

        mLastFrameTime = currentTime;
    }
    #ifdef GL_USE_OPENGL_LATEST
        ImGui_ImplOpenGL3_Shutdown();
    #else 
        ImGui_ImplOpenGL2_Shutdown();
    #endif
        ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
