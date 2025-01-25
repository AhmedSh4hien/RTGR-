#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h> // holds all OpenGL type declarations
#include <GLFW/glfw3.h>

//#include <nanogui/nanogui.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <optional>

GLFWwindow *window = nullptr;
bool gui = false;

// utility function to derminate the window
// ---------------------------------------------------
void DestroyWindow(void)
{
    // if (screen) delete screen;
    if (window)
    {
        glfwTerminate(); // delete window;
    }
}

// utility function to instantiate a GLFW3 window
// ---------------------------------------------------
int InitWindow(int &width, int &height, const char *appname = "OpenGL", bool resizeable = true)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

    // glsl_version = "#version 440";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RESIZABLE, resizeable);        // make the window resizeable
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, resizeable); // scale to monitor DPI

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(width, height, appname, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &width, &height); // retrieve current size of the window

    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    return 0; // success
}

// utility function to instantiate a GLFW3 window and a GUI
// ---------------------------------------------------
int InitWindowAndGUI(int &width, int &height, const char *appname = "OpenGL", bool resizeable = true)
{
    if (InitWindow(width, height, appname, resizeable) >= 0)
    {

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        //  enable high-dip support

// if docking-experimental of imgui is installed
#ifdef IMGUI_HAS_DOCK
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts; //| ImGuiConfigFlags_DpiEnableScaleViewports;
#endif

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        const char *glsl_version = "#version 430";
        ImGui_ImplOpenGL3_Init(glsl_version);

        /*
        // if docking-experimental of imgui is installed
        #ifdef IMGUI_HAS_DOCK
                glfwSetWindowContentScaleCallback(window,
                                                  [](GLFWwindow *window, float xscale, float yscale)
                                                  {
                                                      ImGuiIO &io = ImGui::GetIO();
                                                      io.FontGlobalScale = std::max<float>(xscale, yscale);
                                                  });
        #endif
        */

        gui = true;
    }
    else
        return -1;

    return 0; // success
}

bool firstUpdate = true;
void UpdateWindow(float deltaTime = 0.0f)
{

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
}

GLFWframebuffersizefun global_fbsize_fun;
void SetFramebufferSizeCallback(GLFWframebuffersizefun fun)
{
    global_fbsize_fun = fun;
    glfwSetFramebufferSizeCallback(window,
                                   [](GLFWwindow *window_, int width, int height)
                                   {
                                       global_fbsize_fun(window_, width, height);
                                   });
}

GLFWcursorposfun global_cursorpos_fun;
void SetCursorPosCallback(GLFWcursorposfun fun)
{

    global_cursorpos_fun = fun;
    if (!gui)
    {
        glfwSetCursorPosCallback(window, global_cursorpos_fun);
    }
    else
    {
        glfwSetCursorPosCallback(
            window,
            [](GLFWwindow *window_, double x, double y)
            {
                // ALWAYS forward mouse data to ImGui! This is automatic with default backends.
                ImGuiIO &io = ImGui::GetIO();
                io.AddMousePosEvent((float)x, (float)y);

                // ONLY forward mouse data to your underlying app/game, if ImGui did not capture it.
                if (!io.WantCaptureMouse)
                    global_cursorpos_fun(window_, x, y);
            });
    }
}

GLFWscrollfun global_scroll_fun;
void SetScrollCallback(GLFWscrollfun fun)
{
    global_scroll_fun = fun;
    if (!gui)
    {
        glfwSetScrollCallback(window, global_scroll_fun);
    }
    else
    {
        glfwSetScrollCallback(
            window,
            [](GLFWwindow *window_, double x, double y)
            {
                // ALWAYS forward mouse data to ImGui! This is automatic with default backends.
                ImGuiIO &io = ImGui::GetIO();
                io.AddMouseWheelEvent((float)x, (float)y);

                // ONLY forward mouse data to your underlying app/game, if ImGui did not capture it.
                if (!io.WantCaptureMouse)
                    global_scroll_fun(window_, x, y);
            });
    }
}

GLFWmousebuttonfun global_mousbutton_fun;
void SetMouseButtonCallback(GLFWmousebuttonfun fun)
{
    global_mousbutton_fun = fun;
    if (!gui)
    {
        glfwSetMouseButtonCallback(window, global_mousbutton_fun);
    }
    else
    {
        glfwSetMouseButtonCallback(
            window,
            [](GLFWwindow *w_, int button, int action, int modifiers)
            {
                // ALWAYS forward mouse data to ImGui! This is automatic with default backends.
                ImGuiIO &io = ImGui::GetIO();
                if (button >= 0 && button < ImGuiMouseButton_COUNT)
                    io.AddMouseButtonEvent(button, action == GLFW_PRESS);

                // ONLY forward mouse data to your underlying app/game, if ImGui did not capture it.
                if (!io.WantCaptureMouse)
                    global_mousbutton_fun(w_, button, action, modifiers);
            });
    }
}

#endif
