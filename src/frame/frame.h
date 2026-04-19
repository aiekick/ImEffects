#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <ImNodal/ImNodal.h>
#include <ImGenie/ImGenie.h>
#include <ImCoolBar/ImCoolBar.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>

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
        ImGenieParams genieSettings{};
        ImGenieParams genieDefaultSettings{};
        ImGui::ImCoolBarSettings coolBarSettings;
        ImGui::ImCoolBarSettings coolBarDefaultSettings;
        ImNodal::CanvasSettings canvasSettings;
        ImNodal::GraphSettings graphSettings;

        // Demo graph data (M1) — user-retained positions; ImNodal just echoes.
        ImVec2 nodeAPos{-200.0f, -60.0f};
        ImVec2 nodeBPos{ 180.0f, -20.0f};
        float  nodeA_valueA{1.0f};
        float  nodeA_valueB{2.0f};
        float  nodeB_multiplier{0.5f};
        bool   nodalContextCreated{false};

        // Demo links (M2) — user-retained: ImNodal draws them from slot IDs.
        struct LinkRecord {
            uint64_t id;
            uint64_t from;
            uint64_t to;
        };
        std::vector<LinkRecord> links{};
        uint64_t nextLinkId{1000};

        // Demo reroutes (M2) — created by double-clicking a link.
        struct RerouteRecord {
            uint64_t nodeId;
            uint64_t slotId;
            ImVec2   pos;
        };
        std::vector<RerouteRecord> reroutes{};
        uint64_t nextRerouteNodeId{3000};
        uint64_t nextRerouteSlotId{4000};

        // Standalone slot demo (M2) — two linkable slots in a plain ImGui window,
        // no BeginCanvas / BeginGraph involved.
        std::vector<LinkRecord> freeLinks{};
        uint64_t nextFreeLinkId{5000};
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
    void m_drawGraph();
    void m_drawDialogs();
    void m_drawMainMenubar();
    const char* getWindowNameFromIcon(const IconEntry& aIcon) const;
};
