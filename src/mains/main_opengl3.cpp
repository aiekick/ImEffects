// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui.h>
#include <imgui_internal.h>

#include <GLFW/glfw3.h>

#include <src/frame/frame.h>

#include <3rdparty/imgui/backends/imgui_impl_opengl3.h>
#include <3rdparty/imgui/backends/imgui_impl_glfw.h>

#include <cstdio>
#include <sstream>
#include <fstream>
#include <clocale>
#include <string>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <src/helpers/stb_image.h>

#include <ImGenie/ImGenie.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <GLES3/gl3.h>
#else
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>  // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>  // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow* pWindow{nullptr};
Frame frame;
ImVec4 viewportRect;

#ifdef __EMSCRIPTEN__
extern "C" {
EMSCRIPTEN_KEEPALIVE void sLoadIniSettingsFromDisk() {
    ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);
}
}
#endif

static void sUpdate(void*) {
    int display_w{};
    int display_h{};
    ImGuiIO& io = ImGui::GetIO();

#ifndef __EMSCRIPTEN__
    glfwPollEvents();
#else
    static int frameCount = 0;
    if (++frameCount % 60 == 0) {
        EM_ASM(FS.syncfs(false, function(err){}));
    }
#endif

    glfwGetFramebufferSize(pWindow, &display_w, &display_h);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    try {
        frame.update(ImVec2(display_w, display_h));
    } catch (std::exception& ex) {
        std::cout << "exception catched with message : " << ex.what() << std::endl;
        assert(0);
    }

    // Cpu Zone : prepare
    ImGui::Render();

    // Capture windows to FBO before main render
    ImGenie::Capture();

    // GPU Zone : Rendering
    glfwMakeContextCurrent(pWindow);

    glViewport(0, 0, display_w, display_h);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(pWindow);
}

int main(int, char**) {
#ifdef _MSC_VER
    // active memory leak detector
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    auto loc = std::setlocale(LC_ALL, ".UTF8");
    if (!loc) {
        printf("setlocale fail to apply with this compiler. it seems the unicode will be NOK\n");
    }
	std::setlocale(LC_NUMERIC, "C");

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

#ifdef __EMSCRIPTEN__
    // GL ES 3.0 + GLSL 130
    const char* glsl_version = "#version 300 es";
#else
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
#endif

    // Create window with graphics context
    pWindow = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (pWindow == nullptr)
        return 1;
    glfwMakeContextCurrent(pWindow);
#ifndef __EMSCRIPTEN__
    glfwSwapInterval(1);  // Enable vsync (Emscripten handles this via requestAnimationFrame)
#endif

#ifndef __EMSCRIPTEN__
// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false;  // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return EXIT_FAILURE;
    }
#endif

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    IMGENIE_CHECKVERSION();
    ImGenie::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking
    io.FontAllowUserScaling = true;  // zoom wiht ctrl + mouse wheel

#ifdef __EMSCRIPTEN__
    io.IniFilename = "/settings/imgui.ini";
    EM_ASM(FS.mkdir('/settings'); FS.mount(IDBFS, {}, '/settings'); FS.syncfs(
        true, function(err) {
            if (err)
                console.log('IDBFS load error:', err);
            else
                Module.ccall('sLoadIniSettingsFromDisk', 'void', [], []);
        }););
#endif

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGenie::SetCreateCaptureFunc([](int32_t aWidth, int32_t aHeight, ImDrawData* apDrawData) {
        ImTextureRef ret{};

        // Create texture for FBO color attachment
        GLuint fboTex{};
        glGenTextures(1, &fboTex);
        glBindTexture(GL_TEXTURE_2D, fboTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, aWidth, aHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Create FBO and attach texture
        GLuint fbo{};
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &fbo);
            glDeleteTextures(1, &fboTex);
            return ret;
        }

        // Save GL state, render to FBO, restore
        GLint prevViewport[4];
        glGetIntegerv(GL_VIEWPORT, prevViewport);
        glViewport(0, 0, aWidth, aHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(apDrawData);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

        // Delete FBO but keep the texture (it contains the rendered snapshot)
        glDeleteFramebuffers(1, &fbo);

        ret._TexID = fboTex;
        return ret;
    });
    ImGenie::SetCaptureFlipV(true);  // OpenGL Y-axis is inverted

    ImGenie::SetDestroyCaptureFunc([](const ImTextureRef& aTex) {
        GLuint texID = static_cast<GLuint>(static_cast<uintptr_t>(aTex._TexID));
        glDeleteTextures(1, &texID);
    });

    // Setup frame texture callbacks
    frame.setLoadTextureFunc([](const uint8_t* aData, int32_t aWidth, int32_t aHeight, int32_t aChannels) {
        ImTextureRef ret{};
        GLuint texId = 0U;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (aChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, aWidth, aHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, aData);
        } else if (aChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, aWidth, aHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, aData);
        }
        glFinish();
        glBindTexture(GL_TEXTURE_2D, 0);
        ret._TexID = texId;
        return ret;
    });
    frame.setDestroyTextureFunc([](const ImTextureRef& aTex) {
        GLuint texID = static_cast<GLuint>(static_cast<uintptr_t>(aTex._TexID));
        glDeleteTextures(1, &texID);
    });

    if (!frame.init()) {
        return EXIT_FAILURE;
    }

    ImVec4 viewportRect;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(sUpdate, nullptr, 0, true);
#else
    while (!glfwWindowShouldClose(pWindow)) {
        sUpdate(nullptr);
    }
#endif

    frame.unit();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGenie::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(pWindow);
    glfwTerminate();

    return 0;
}
