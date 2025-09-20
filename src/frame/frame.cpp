#include "frame.h"

#include <array>

#include <src/helpers/stb_image.h>

#include <src/helpers/RenderDocController.h>

bool Frame::init() {
    const std::array<std::string, 11> icons_name = {
        "Settings",   //
        "Activa",     //
        "Magnet",     //
        "Blender",    //
        "CLion",      //
        "Firefox",    //
        "Gimp",       //
        "Godot",      //
        "VLC",        //
        "Wikipedia",  //
        "GeoGebra"    //
    };
    m_datas.bgRef = m_loadTexture("res/desert.jpg");
    for (const auto& name : icons_name) {
        IconEntry entry;
        entry.name = name;
        entry.texRef = m_loadTexture("res/" + name + ".png");
        m_datas.icons.push_back(entry);
    }
    m_datas.coolBarSettings.normalSize = 40.0f;
    m_datas.coolBarSettings.hoveredSize = 150.0f;
    m_datas.coolBarSettings.anchor = ImVec2(0.5f, 1.0f);
    m_datas.coolBarSettings.animStep = 0.05f;
#ifdef ENABLE_IMCOOLBAR_DEBUG
    m_datas.coolBarSettings.animStep = 0.005f;
#endif
    m_datas.coolBarDefaultSettings = m_datas.coolBarSettings;
    m_datas.genieDefaultSettings = m_datas.genieSettings;
    return true;
}

void Frame::unit() {
    if (m_destroyTextureFunc) {
        for (auto& icon : m_datas.icons) {
            m_destroyTextureFunc(icon.texRef);
        }
        m_destroyTextureFunc(m_datas.bgRef);
    }
}

void Frame::update(const ImVec2& arDisplaySize) {
    m_drawBackground(m_datas.bgRef, arDisplaySize);
    m_drawDialogs();
    m_drawBar();
}

ImTextureRef Frame::m_loadTexture(const std::string& arFilePathName) {
    ImTextureRef ret{};
    if (!m_loadTextureFunc) {
        return ret;
    }
    int32_t w{}, h{}, chans{};
    auto* image = stbi_load(arFilePathName.c_str(), &w, &h, &chans, 0);
    if (image) {
        stbi_image_free(image);
        if (chans == 3) {
            image = stbi_load(arFilePathName.c_str(), &w, &h, &chans, STBI_rgb);
        } else if (chans == 4) {
            image = stbi_load(arFilePathName.c_str(), &w, &h, &chans, STBI_rgb_alpha);
        }
    }
    if (image) {
        ret = m_loadTextureFunc(reinterpret_cast<const uint8_t*>(image), w, h, chans);
        stbi_image_free(image);
    }
    return ret;
}

void Frame::m_drawBackground(const ImTextureRef& arTexRef, const ImVec2& arDisplaySize) {
    auto* const pDrawList = ImGui::GetBackgroundDrawList();
    pDrawList->AddImage(arTexRef, ImVec2(0, 0), arDisplaySize);
}

void Frame::m_drawBar() {
    ImGui::SetNextWindowBgAlpha(0.0f);
    bool opened = ImGui::BeginCoolBar("Dock", m_datas.coolBarSettings);
    if (opened) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2());
        size_t idx = 0U;
        for (size_t i = 0; i < m_datas.icons.size(); ++i) {
            auto& icon = m_datas.icons[i];
            if (ImGui::CoolBarItem()) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
                float w = ImGui::GetCoolBarItemWidth();
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::ImageButton("##icon", icon.texRef, ImVec2(w, w))) {
                    icon.show = !icon.show;
#ifndef __EMSCRIPTEN__
                    RenderDocController::Instance()->RequestCapture();
#endif  // __EMSCRIPTEN__
                }
                icon.hovered = ImGui::IsItemHovered();
                ImGui::PopID();
                icon.buttonRect = GImGui->LastItemData.Rect;
                const auto* pWinName = getWindowNameFromIcon(icon);
                if (ImGenie::IsEffectActive(pWinName)) {
                    auto* const pDrawList = ImGui::GetForegroundDrawList();
                    pDrawList->AddImage(icon.texRef, icon.buttonRect.Min, icon.buttonRect.Max);
                }
                const auto& center = icon.buttonRect.GetCenter();
                const float shrink = 0.25f;
                icon.buttonRect.Min = ImVec2(center.x - w * shrink, center.y - w * shrink);
                icon.buttonRect.Max = ImVec2(center.x + w * shrink, center.y + w * shrink);
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(2);
            }
        }
        ImGui::PopStyleVar();
        ImGui::EndCoolBar();
    }
}

void Frame::m_drawDialogs() {
    for (auto& icon : m_datas.icons) {
        const auto* pWinName = getWindowNameFromIcon(icon);
        if (icon.name == "Settings" || icon.name == "Activa" || icon.name == "Magnet" || icon.name == "GeoGebra") {
            // These windows manage their own Begin/End, use Allow() manually
            if (ImGenie::Allow(pWinName, icon.buttonRect, &icon.show, &m_datas.genieSettings)) {
                if (icon.show) {
                    if (icon.name == "Settings") {
                        ImGui::ShowDemoWindow(&icon.show);
                    } else if (icon.name == "Activa") {
                        ImGui::ShowMetricsWindow(&icon.show);
                    } else if (icon.name == "Magnet") {
                        ImGui::ShowCoolBarDemoWindow(&icon.show, &m_datas.coolBarSettings, &m_datas.coolBarDefaultSettings);
                    } else if (icon.name == "GeoGebra") {
                        ImGenie::ShowDemoWindow(&icon.show, &m_datas.genieSettings, &m_datas.genieDefaultSettings);
                    }
                }
            }
        } else {
            // Normal windows: use the Begin/End wrapper
            if (ImGenie::Begin(icon.name.c_str(), icon.buttonRect, &icon.show, ImGuiWindowFlags_None, &m_datas.genieSettings)) {
                ImGui::Image(icon.texRef, ImVec2(256, 256));
                ImGenie::End();
            }
        }
    }
}

const char* Frame::getWindowNameFromIcon(const IconEntry& aIcon) const {
    const char* pWinName = aIcon.name.c_str();
    if (aIcon.name == "Settings") {
        pWinName = "Dear ImGui Demo";
    } else if (aIcon.name == "Activa") {
        pWinName = "Dear ImGui Metrics/Debugger";
    } else if (aIcon.name == "Magnet") {
        pWinName = "ImCoolBar Demo";
    } else if (aIcon.name == "GeoGebra") {
        pWinName = "ImGenie Demo";
    }
    return pWinName;
}
