#pragma once

#ifndef __EMSCRIPTEN__

#pragma warning(disable : 4251)

#include "RenderDocApp.h"

class RenderDocController {
private:
    RENDERDOC_API_1_0_0* m_RDdocPtr = nullptr;
    pRENDERDOC_GetAPI m_GetApiPtr = nullptr;

    int32_t m_Capture_FramesLeft = 0;
    bool m_Capture_Started = false;

public:
    bool Init();
    void Unit();

    void RequestCapture(int32_t aFrameCount = 1);
    bool IsCaptureRequested();
    bool IsCaptureStarted();

    void StartCaptureIfResquested();
    void EndCaptureIfResquested();

public:
    static RenderDocController* Instance(RenderDocController* vCopy = nullptr, bool vForce = false) {
        static RenderDocController _instance;
        static RenderDocController* _instance_copy = nullptr;
        if (vCopy || vForce)
            _instance_copy = vCopy;
        if (_instance_copy)
            return _instance_copy;
        return &_instance;
    }

protected:
    RenderDocController() = default;                            // Prevent construction
    RenderDocController(const RenderDocController&) = default;  // Prevent construction by copying
    RenderDocController& operator=(const RenderDocController&) {
        return *this;
    };                                 // Prevent assignment
    virtual ~RenderDocController() = default;  // Prevent unwanted destruction
};

#endif // __EMSCRIPTEN__
