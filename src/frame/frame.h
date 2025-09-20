#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <ImGenie/ImGenie.h>
#include <ImCoolBar/ImCoolBar.h>

class Frame {
public:
    // Backend-agnostic texture loading/destroying callbacks
    using LoadTextureFunc = std::function<ImTextureRef(const uint8_t* aData, int32_t aWidth, int32_t aHeight, int32_t aChannels)>;
    using DestroyTextureFunc = std::function<void(const ImTextureRef& aTexRef)>;

private:
    struct IconEntry {
        std::string name{};
        ImTextureRef texRef{};
        bool show{false};
        bool hovered{false};
        ImRect buttonRect{};
    };
    struct AppDatas {
        std::vector<IconEntry> icons{};
        ImTextureRef bgRef{};
        ImGenieSettings genieSettings{};
        ImGenieSettings genieDefaultSettings{};
        ImGui::ImCoolBarSettings coolBarSettings;
        ImGui::ImCoolBarSettings coolBarDefaultSettings;
    } m_datas;

    LoadTextureFunc m_loadTextureFunc{};
    DestroyTextureFunc m_destroyTextureFunc{};

public:
    void setLoadTextureFunc(const LoadTextureFunc& arFunc) {
        m_loadTextureFunc = arFunc;
    }
    void setDestroyTextureFunc(const DestroyTextureFunc& arFunc) {
        m_destroyTextureFunc = arFunc;
    }
    bool init();
    void unit();
    void update(const ImVec2& arDisplaySize);

private:
    ImTextureRef m_loadTexture(const std::string& arFilePathName);
    void m_drawBackground(const ImTextureRef& arTexRef, const ImVec2& arDisplaySize);
    void m_drawBar();
    void m_drawDialogs();
    const char* getWindowNameFromIcon(const IconEntry& aIcon) const;
};
