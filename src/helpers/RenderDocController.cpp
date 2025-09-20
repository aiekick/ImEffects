
#ifndef __EMSCRIPTEN__

#include "RenderDocController.h"

#if defined(WIN32)
#include <Windows.h>
#endif  // WIN32

#include <cstdio>

bool RenderDocController::Init() {
    m_GetApiPtr = nullptr;

#if defined(WIN32)
    HMODULE mod = GetModuleHandleA("renderdoc.dll");
    if (mod) {
        m_GetApiPtr = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
    }
#elif defined(__linux__)
    void* mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD);
    if (mod) {
        m_GetApiPtr = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
    }
#elif defined(__APPLE__)
    void* mod = dlopen("librenderdoc.dylib", RTLD_NOW | RTLD_NOLOAD);
    if (mod) {
        m_GetApiPtr = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
    }
#else
#error UNKNOWN PLATFORM
#endif

    if (m_GetApiPtr) {
        int ret = m_GetApiPtr(eRENDERDOC_API_Version_1_0_0, (void**)&m_RDdocPtr);
        if (ret != 1) {
            m_RDdocPtr = nullptr;
        } else {
            int major, minor, patch;
            m_RDdocPtr->GetAPIVersion(&major, &minor, &patch);
            printf("-----------\n");
            printf("Renderdoc DLL Loaded\n");
            printf("Renderdoc Api Version : %i.%i.%i\n", major, minor, patch);
            printf("-----------\n");
        }
    }

    return true;
}

void RenderDocController::Unit() {
    m_RDdocPtr = nullptr;
    m_GetApiPtr = nullptr;
}

void RenderDocController::RequestCapture(int32_t aFrameCount) {
    m_Capture_FramesLeft = aFrameCount;
}

bool RenderDocController::IsCaptureRequested() {
    return m_Capture_FramesLeft > 0;
}

bool RenderDocController::IsCaptureStarted() {
    return m_Capture_Started;
}

void RenderDocController::StartCaptureIfResquested() {
    if (m_Capture_FramesLeft <= 0)
        return;

    m_Capture_Started = true;

    if (m_RDdocPtr) {
        printf("Renderdoc Capture Started (frames left: %i)\n", m_Capture_FramesLeft);
        m_RDdocPtr->StartFrameCapture(NULL, NULL);
    }
}

void RenderDocController::EndCaptureIfResquested() {
    if (!m_Capture_Started)
        return;

    // stop the capture
    if (m_RDdocPtr) {
        m_RDdocPtr->EndFrameCapture(NULL, NULL);
        printf("Renderdoc Capture Ended (frames left: %i)\n", m_Capture_FramesLeft - 1);
    }

    --m_Capture_FramesLeft;
    if (m_Capture_FramesLeft <= 0) {
        m_Capture_FramesLeft = 0;
        m_Capture_Started = false;
    }
}

#endif // __EMSCRIPTEN__
