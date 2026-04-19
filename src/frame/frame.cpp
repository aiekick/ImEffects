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
    // --- Main ImNodal demo window (canvas + graph with 2 nodes + links) ---
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

                // --- Reroutes (spawned on link double-click below) ---
                for (auto& rr : m_datas.reroutes) {
                    if (ImNodal::BeginRerouteNode(rr.nodeId, rr.slotId, &rr.pos)) {
                        ImNodal::EndRerouteNode();
                    }
                }

                // --- Render existing links + detect double-click to split ---
                struct PendingSplit { size_t linkIndex; ImVec2 pos; };
                std::vector<PendingSplit> pendingSplits;
                for (size_t i = 0; i < m_datas.links.size(); ++i) {
                    const auto& l = m_datas.links[i];
                    ImNodal::Link(l.id, l.from, l.to);
                    if (ImNodal::IsLinkDoubleClicked(l.id)) {
                        // Mouse pos is in canvas-local space inside BeginGraph.
                        pendingSplits.push_back({i, ImGui::GetIO().MousePos});
                    }
                }

                // --- Connection creation (thedmd-style flow) ---
                if (ImNodal::BeginConnectionCreate()) {
                    ImNodal::Id from = 0, to = 0;
                    if (ImNodal::QueryNewLink(&from, &to)) {
                        // Rule: endpoints must not both be inputs or both outputs.
                        // InOut slots match anything.
                        const auto rFrom = ImNodal::GetSlotRole(from);
                        const auto rTo   = ImNodal::GetSlotRole(to);
                        const bool ok =
                            (rFrom == ImNodal::SlotRole_InOut) ||
                            (rTo   == ImNodal::SlotRole_InOut) ||
                            (rFrom != rTo);
                        if (ok) {
                            if (ImNodal::AcceptNewLink()) {
                                if (rFrom == ImNodal::SlotRole_Input && rTo == ImNodal::SlotRole_Output) {
                                    std::swap(from, to);
                                }
                                m_datas.links.push_back({m_datas.nextLinkId++, from, to});
                            }
                        } else {
                            ImNodal::RejectNewLink("same direction");
                        }
                    }
                    ImNodal::EndConnectionCreate();
                }

                // --- Apply pending link splits (from double-click) ---
                for (auto it = pendingSplits.rbegin(); it != pendingSplits.rend(); ++it) {
                    if (it->linkIndex >= m_datas.links.size()) continue;
                    const auto old = m_datas.links[it->linkIndex];
                    AppDatas::RerouteRecord rr;
                    rr.nodeId = m_datas.nextRerouteNodeId++;
                    rr.slotId = m_datas.nextRerouteSlotId++;
                    rr.pos    = it->pos;
                    m_datas.reroutes.push_back(rr);
                    m_datas.links.erase(m_datas.links.begin() + (ptrdiff_t)it->linkIndex);
                    m_datas.links.push_back({m_datas.nextLinkId++, old.from,  rr.slotId});
                    m_datas.links.push_back({m_datas.nextLinkId++, rr.slotId, old.to});
                }

                // --- Delete key: remove selected link, OR selected reroute + its links ---
                if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                    const ImNodal::Id selLink = ImNodal::GetSelectedLink();
                    const ImNodal::Id selNode = ImNodal::GetSelectedNode();
                    if (selLink != 0) {
                        for (auto lit = m_datas.links.begin(); lit != m_datas.links.end(); ++lit) {
                            if (lit->id == selLink) { m_datas.links.erase(lit); break; }
                        }
                        ImNodal::SetSelectedLink(0);
                    } else if (selNode != 0) {
                        // If the selected node is a reroute, drop all links touching
                        // its slot, then drop the reroute itself.
                        for (auto rit = m_datas.reroutes.begin(); rit != m_datas.reroutes.end(); ++rit) {
                            if (rit->nodeId != selNode) continue;
                            const uint64_t sid = rit->slotId;
                            for (auto lit = m_datas.links.begin(); lit != m_datas.links.end(); ) {
                                if (lit->from == sid || lit->to == sid) {
                                    lit = m_datas.links.erase(lit);
                                } else {
                                    ++lit;
                                }
                            }
                            m_datas.reroutes.erase(rit);
                            ImNodal::SetSelectedNode(0);
                            break;
                        }
                    }
                }

                ImNodal::EndGraph();
            }
            ImNodal::EndCanvas();
        }
        ImGenie::End();
    }

    // --- Bonus : standalone slots in a plain ImGui window (no BeginGraph) ---
    // Two linkable slots living entirely outside of any canvas/graph; the
    // connection-create state machine and Link() work transparently in screen
    // space.
    if (ImGui::Begin("Free Slot Demo")) {
        ImGui::TextUnformatted("Slots outside any graph — draggable + linkable:");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        // Source slot on the left, target slot on the right, separated by a
        // spacer so there's visual room for the link to curve.
        if (ImNodal::BeginOutputSlot(9001, "Source")) {
            ImNodal::EndSlot();
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(160.0f, 0.0f));
        ImGui::SameLine();
        if (ImNodal::BeginInputSlot(9002, "Target")) {
            ImNodal::EndSlot();
        }

        // Render existing free-window links.
        for (const auto& l : m_datas.freeLinks) {
            ImNodal::Link(l.id, l.from, l.to);
        }

        // Same connection flow as the graph demo.
        if (ImNodal::BeginConnectionCreate()) {
            ImNodal::Id f = 0, t = 0;
            if (ImNodal::QueryNewLink(&f, &t)) {
                const auto rF = ImNodal::GetSlotRole(f);
                const auto rT = ImNodal::GetSlotRole(t);
                const bool ok = (rF == ImNodal::SlotRole_InOut) || (rT == ImNodal::SlotRole_InOut) || (rF != rT);
                if (ok) {
                    if (ImNodal::AcceptNewLink()) {
                        if (rF == ImNodal::SlotRole_Input && rT == ImNodal::SlotRole_Output) std::swap(f, t);
                        m_datas.freeLinks.push_back({m_datas.nextFreeLinkId++, f, t});
                    }
                } else {
                    ImNodal::RejectNewLink("same direction");
                }
            }
            ImNodal::EndConnectionCreate();
        }

        // Delete key removes the standalone-selected link (uses the context-level
        // selection, since we're outside any graph).
        if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            const ImNodal::Id sel = ImNodal::GetSelectedLink();
            if (sel != 0) {
                for (auto lit = m_datas.freeLinks.begin(); lit != m_datas.freeLinks.end(); ++lit) {
                    if (lit->id == sel) { m_datas.freeLinks.erase(lit); break; }
                }
                ImNodal::SetSelectedLink(0);
            }
        }

        ImGui::Separator();
        ImGui::TextUnformatted("M2 status:");
        ImGui::Text("Selected node: %llu", (unsigned long long)ImNodal::GetSelectedNode());
        ImGui::Text("Dragging A: %d", (int)ImNodal::IsNodeDragging(100));
        ImGui::Text("Dragging B: %d", (int)ImNodal::IsNodeDragging(200));
        ImGui::Text("Graph links: %d", (int)m_datas.links.size());
        ImGui::Text("Free links: %d", (int)m_datas.freeLinks.size());
        ImGui::Text("Reroutes: %d", (int)m_datas.reroutes.size());
        if (ImGui::SmallButton("Clear all links")) {
            m_datas.links.clear();
            m_datas.freeLinks.clear();
            m_datas.reroutes.clear();
        }
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
