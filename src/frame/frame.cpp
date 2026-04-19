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
    // ImNodal context (auto-becomes current if none exists)
    ImNodal::CreateContext();
    m_datas.nodalContextCreated = true;
    return true;
}

void Frame::unit() {
    if (m_destroyTextureFunc) {
        for (auto& icon : m_datas.icons) {
            m_destroyTextureFunc(icon.texRef);
        }
        m_destroyTextureFunc(m_datas.bgRef);
    }
    if (m_datas.nodalContextCreated) {
        ImNodal::DestroyContext();
        m_datas.nodalContextCreated = false;
    }
}

void Frame::update(const ImVec2& arDisplaySize) {
    m_drawBackground(m_datas.bgRef, arDisplaySize);
    m_drawMainMenubar();
    m_drawDialogs();
    m_drawGraph();
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

void Frame::m_drawGraph() {
    // --- Main ImNodal demo window (canvas + graph with 2 nodes) ---
    if (ImGenie::Begin("ImNodal", nullptr, ImGuiWindowFlags_None, &m_datas.genieSettings)) {
        m_datas.canvasSettings.drawGrid = true;
        if (ImNodal::BeginCanvas("ImNodal", ImVec2(0, 0), m_datas.canvasSettings)) {
            if (ImNodal::BeginGraph(1 /* root graph id */, m_datas.graphSettings)) {
                // --- Node A : Add (A + B => Result) ---
                if (ImNodal::BeginNode(100 /* id */, &m_datas.nodeAPos)) {
                    if (ImNodal::BeginHeader()) {
                        ImGui::TextUnformatted("Add");
                        ImNodal::EndHeader();
                    }
                    if (ImNodal::BeginInputs()) {
                        if (ImNodal::BeginInputSlot(101, "A")) {
                            ImGui::SetNextItemWidth(60.0f);
                            ImGui::DragFloat("##A", &m_datas.nodeA_valueA, 0.01f);
                            ImNodal::EndSlot();
                        }
                        if (ImNodal::BeginInputSlot(102, "B")) {
                            ImGui::SetNextItemWidth(60.0f);
                            ImGui::DragFloat("##B", &m_datas.nodeA_valueB, 0.01f);
                            ImNodal::EndSlot();
                        }
                        ImNodal::EndInputs();
                    }
                    if (ImNodal::BeginCenter()) {
                        ImGui::Text("= %.2f", m_datas.nodeA_valueA + m_datas.nodeA_valueB);
                        ImNodal::EndCenter();
                    }
                    if (ImNodal::BeginOutputs()) {
                        if (ImNodal::BeginOutputSlot(103, "Result")) {
                            ImNodal::EndSlot();
                        }
                        ImNodal::EndOutputs();
                    }
                    if (ImNodal::BeginFooter()) {
                        ImGui::TextDisabled("id=100");
                        ImNodal::EndFooter();
                    }
                    ImNodal::EndNode();
                }

                // --- Node B : Scale (X => Y) ---
                if (ImNodal::BeginNode(200, &m_datas.nodeBPos)) {
                    if (ImNodal::BeginHeader()) {
                        ImGui::TextUnformatted("Scale");
                        ImNodal::EndHeader();
                    }
                    if (ImNodal::BeginInputs()) {
                        if (ImNodal::BeginInputSlot(201, "X")) {
                            ImNodal::EndSlot();
                        }
                        ImNodal::EndInputs();
                    }
                    if (ImNodal::BeginCenter()) {
                        ImGui::SetNextItemWidth(80.0f);
                        ImGui::DragFloat("k", &m_datas.nodeB_multiplier, 0.01f);
                        ImNodal::EndCenter();
                    }
                    if (ImNodal::BeginOutputs()) {
                        if (ImNodal::BeginOutputSlot(202, "Y")) {
                            ImNodal::EndSlot();
                        }
                        ImNodal::EndOutputs();
                    }
                    ImNodal::EndNode();
                }

                ImNodal::EndGraph();
            }
            ImNodal::EndCanvas();
        }
        ImGenie::End();
    }

    // --- Bonus : a standalone slot in a plain ImGui window (no BeginGraph) ---
    // Verifies that the slot primitive works outside of any graph context.
    if (ImGui::Begin("Free Slot Demo")) {
        ImGui::TextUnformatted("A slot outside any graph:");
        if (ImNodal::BeginInputSlot(999, "Var")) {
            ImNodal::EndSlot();
        }
        ImGui::Separator();
        ImGui::TextUnformatted("M1 status:");
        ImGui::Text("Selected node: %llu", (unsigned long long)ImNodal::GetSelectedNode());
        ImGui::Text("Dragging A: %d", (int)ImNodal::IsNodeDragging(100));
        ImGui::Text("Dragging B: %d", (int)ImNodal::IsNodeDragging(200));
    }
    ImGui::End();
}


void Frame::m_drawDialogs() {
    for (auto& icon : m_datas.icons) {
        const auto* pWinName = getWindowNameFromIcon(icon);
        // Update genie destRect from button position each frame
        const auto& br = icon.buttonRect;
        m_datas.genieSettings.transitions.genie.destRect = {br.Min.x, br.Min.y, br.Max.x, br.Max.y};
        if (icon.name == "Settings" || icon.name == "Activa" || icon.name == "Magnet" || icon.name == "GeoGebra") {
            // These windows manage their own Begin/End, use Allow() manually
            if (ImGenie::Allow(pWinName, &icon.show, &m_datas.genieSettings)) {
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
            if (ImGenie::Begin(icon.name.c_str(), &icon.show, ImGuiWindowFlags_None, &m_datas.genieSettings)) {
                ImGui::Image(icon.texRef, ImVec2(256, 256));
                ImGenie::End();
            }
        }
    }

    // ImGuiFileDialog
    bool opened = ImGuiFileDialog::Instance()->IsOpened("OpenDlg");
    m_datas.genieSettings.transitions.genie.destRect = {};
    if (ImGenie::Allow("Open Dialog##OpenDlg", &opened, &m_datas.genieSettings)) {
        if (opened) {
            if (ImGuiFileDialog::Instance()->Display("OpenDlg")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                }
                ImGuiFileDialog::Instance()->Close();
            }
        }
    }
}

void Frame::m_drawMainMenubar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(" Dialogs")) {
            if (ImGui::MenuItem(" Open")) {
                IGFD::FileDialogConfig config;
                config.countSelectionMax = 1;
                ImGuiFileDialog::Instance()->OpenDialog("OpenDlg", "Open Dialog", "All File{((.*))}", config);
            }

            ImGui::EndMenu();
        }

        ImGui::Spacing();

        // ImGui Infos
        static const int sBufLen = 50 + 1;
        static char buf[sBufLen] = "\0";
        ImFormatString(buf, sBufLen, "Dear ImGui %s (Docking)", ImGui::GetVersion());
        const auto size = ImGui::CalcTextSize(buf);

        ImGui::ItemSize(ImVec2(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f, 0.0f));
        ImGui::Text("%s", buf);

        ImGui::EndMainMenuBar();
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
