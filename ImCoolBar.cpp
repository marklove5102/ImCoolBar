/*
MIT License

Copyright (c) 2024-2026 Stephane Cuillerdier (aka Aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImCoolBar.h"
#include "imgui_internal.h"
#include <cmath>
#include <vector>
#include <array>

#define ICB_PREFIX "ICB"
#define ICB_TYPE_MAGIC 0x49434231  // "ICB1"

#ifdef _MSC_VER
#define ICB_DEBUG_BREAK __debugbreak()
#else
#define ICB_DEBUG_BREAK
#endif

#define BREAK_ON_KEY(KEY)         \
    if (ImGui::IsKeyPressed(KEY)) \
    ICB_DEBUG_BREAK

static float s_bubbleEffect(const float aValue, const float aStrength) {
    return powf(fabsf(cosf(aValue * IM_PI * aStrength)), 8.0f);
}

static float s_getBarSize(const float aNormalSize, const float aHoveredSize, const float aScale) {
    return ImClamp(aNormalSize + (aHoveredSize - aNormalSize) * aScale, aNormalSize, aHoveredSize);
}

// https://codesandbox.io/s/motion-dock-forked-hs4p8d?file=/src/Dock.tsx
static float s_getHoverSize(const float aValue, const float aNormalSize, const float aHoveredSize, const float aStrength, const float aScale) {
    return s_getBarSize(aNormalSize, aHoveredSize, s_bubbleEffect(aValue, aStrength) * aScale);
}

static bool s_isWindowHovered(ImGuiWindow* apWindow) {
    return ImGui::IsMouseHoveringRect(apWindow->Rect().Min, apWindow->Rect().Max);
}

static float s_getChannel(const ImVec2& arVec, const ImCoolBarFlags aFlags) {
    if (aFlags & ImCoolBarFlags_Horizontal) {
        return arVec.x;
    }
    return arVec.y;
}

static float s_getChannelInv(const ImVec2& arVec, const ImCoolBarFlags aFlags) {
    if (aFlags & ImCoolBarFlags_Horizontal) {
        return arVec.y;
    }
    return arVec.x;
}

IMCOOLBAR_API bool ImGui::BeginCoolBar(const char* aLabel, ImCoolBarFlags aCBFlags, const ImCoolBarConfig& arConfig, ImGuiWindowFlags aWinFlags) {
    const auto flags =                         //
        aWinFlags                              //
        | ImGuiWindowFlags_NoTitleBar          //
        | ImGuiWindowFlags_NoScrollbar         //
        | ImGuiWindowFlags_AlwaysAutoResize    //
        | ImGuiWindowFlags_NoCollapse          //
        | ImGuiWindowFlags_NoMove              //
        | ImGuiWindowFlags_NoSavedSettings     //
        | ImGuiWindowFlags_NoFocusOnAppearing  //
#ifndef ENABLE_IMCOOLBAR_DEBUG
        | ImGuiWindowFlags_NoBackground  //
#endif                                   //
#ifdef IMGUI_HAS_DOCK
        | ImGuiWindowFlags_DockNodeHost  //
        | ImGuiWindowFlags_NoDocking     //
#endif
        ;
    const auto res = ImGui::Begin(aLabel, nullptr, flags);
    if (!res) {
        ImGui::End();
    } else {
        // Can be Horizontal or Vertical, not both
        const auto isVertical = (aCBFlags & ImCoolBarFlags_Vertical);
        const auto isHorizontal = (aCBFlags & ImCoolBarFlags_Horizontal);
        IM_ASSERT((isHorizontal && !isVertical) || (!isHorizontal && isVertical));
        IM_ASSERT(arConfig.normalSize <= arConfig.hoveredSize);

        auto* const pWindow = GetCurrentWindow();
        pWindow->StateStorage.SetInt(pWindow->GetID(ICB_PREFIX "Type"), ICB_TYPE_MAGIC);
        pWindow->StateStorage.SetInt(pWindow->GetID(ICB_PREFIX "ItemIdx"), 0);
        pWindow->StateStorage.SetInt(pWindow->GetID(ICB_PREFIX "Flags"), aCBFlags);
        const auto anchor = ImClamp(s_getChannelInv(arConfig.anchor, aCBFlags), 0.0f, 1.0f);
        pWindow->StateStorage.SetFloat(pWindow->GetID(ICB_PREFIX "Anchor"), anchor);
        const auto normalSizeId = pWindow->GetID(ICB_PREFIX "NormalSize");
        const auto hoveredSizeId = pWindow->GetID(ICB_PREFIX "HoveredSize");
        pWindow->StateStorage.SetFloat(normalSizeId, arConfig.normalSize);
        pWindow->StateStorage.SetFloat(hoveredSizeId, arConfig.hoveredSize);
        pWindow->StateStorage.SetFloat(pWindow->GetID(ICB_PREFIX "EffectStrength"), arConfig.effectStrength);

        const auto animScaleId = pWindow->GetID(ICB_PREFIX "AnimScale");
        auto animScale = pWindow->StateStorage.GetFloat(animScaleId);
        auto& g = *GImGui;
        const auto animDelta = arConfig.animStep * g.IO.DeltaTime * 60.0f;
        if (s_isWindowHovered(pWindow)) {
            if (animScale < 1.0f) {
                animScale += animDelta;
            }
        } else {
            if (animScale > 0.0f) {
                animScale -= animDelta;
            }
        }

        animScale = ImClamp(animScale, 0.0f, 1.0f);
        pWindow->StateStorage.SetFloat(animScaleId, animScale);

        // --- Position with predicted cross-axis size for THIS frame ---
        const auto pad = ImGui::GetStyle().WindowPadding * 2.0f;
        auto barSize = pWindow->ContentSize + pad;  // along main axis ok
        const auto normalSize = pWindow->StateStorage.GetFloat(normalSizeId);
        const auto hoveredSize = pWindow->StateStorage.GetFloat(hoveredSizeId);
        const auto cross = s_getBarSize(normalSize, hoveredSize, animScale);
        if (isHorizontal) {
            barSize.y = cross + pad.y;
        } else {
            barSize.x = cross + pad.x;
        }

        if (arConfig.anchor.x >= 0.0f && arConfig.anchor.y >= 0.0f) {
            const auto* const pViewport = pWindow->Viewport;
            const auto newPos = ImFloor(pViewport->Pos + (pViewport->Size - barSize) * arConfig.anchor);
            ImGui::SetWindowPos(newPos);
        }
    }

    return res;
}

IMCOOLBAR_API void ImGui::EndCoolBar() {
    ImGui::End();
}

IMCOOLBAR_API bool ImGui::CoolBarItem() {
    auto* const pWindow = GetCurrentWindow();
    if (pWindow->SkipItems) {
        return false;
    }

    const auto itemIndexId = pWindow->GetID(ICB_PREFIX "ItemIdx");
    const auto idx = pWindow->StateStorage.GetInt(itemIndexId);
    const auto coolbarItemId = pWindow->GetID(pWindow->ID + idx + 1);
    auto currentItemSize = pWindow->StateStorage.GetFloat(coolbarItemId);
    const auto cbFlags = pWindow->StateStorage.GetInt(pWindow->GetID(ICB_PREFIX "Flags"));
    const auto animScale = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "AnimScale"));
    const auto normalSize = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "NormalSize"));
    const auto hoveredSize = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "HoveredSize"));
    const auto effectStrength = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "EffectStrength"));
    const auto lastMousePosId = pWindow->GetID(ICB_PREFIX "LastMousePos");
    auto lastMousePos = pWindow->StateStorage.GetFloat(lastMousePosId);

    IM_ASSERT(normalSize > 0.0f);

    if (cbFlags & ImCoolBarFlags_Horizontal) {
        if (idx) {
            ImGui::SameLine();
        }
    }
    auto& g = *GImGui;

    if (s_isWindowHovered(pWindow)) {
        lastMousePos = s_getChannel(g.IO.MousePos, cbFlags);
    } else if (lastMousePos == 0.0f) {
        lastMousePos = s_getChannel(pWindow->Pos, cbFlags) + s_getChannel(pWindow->Size, cbFlags) * 0.5f;
    }

    if (currentItemSize <= 0.0f) {
        currentItemSize = normalSize;
    }

    auto currentSize = normalSize;
    if (animScale > 0.0f) {
        const auto csp = s_getChannel(pWindow->DC.CursorPos, cbFlags);
        const auto ws = s_getChannel(pWindow->Size, cbFlags);
        const auto wp = s_getChannel(g.Style.WindowPadding, cbFlags);
        const auto btnCenter = csp + currentItemSize * 0.5f;
        const auto diffPos = (lastMousePos - btnCenter) / ws;
        currentSize = s_getHoverSize(diffPos, normalSize, hoveredSize, effectStrength, animScale);
        const auto anchor = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "Anchor"));
        const auto barHeight = s_getBarSize(normalSize, hoveredSize, animScale);
        const auto btnOffset = ImFloor((barHeight - currentSize) * anchor + wp);
        if (cbFlags & ImCoolBarFlags_Horizontal) {
            ImGui::SetCursorPosY(btnOffset);
        } else if (cbFlags & ImCoolBarFlags_Vertical) {
            ImGui::SetCursorPosX(btnOffset);
        }
    }

    pWindow->StateStorage.SetInt(itemIndexId, idx + 1);
    pWindow->StateStorage.SetFloat(coolbarItemId, currentSize);
    pWindow->StateStorage.SetFloat(lastMousePosId, lastMousePos);
    pWindow->StateStorage.SetFloat(pWindow->GetID(ICB_PREFIX "ItemCurrentSize"), currentSize);
    pWindow->StateStorage.SetFloat(pWindow->GetID(ICB_PREFIX "ItemCurrentScale"), currentSize / normalSize);

    return true;
}

IMCOOLBAR_API float ImGui::GetCoolBarItemWidth() {
    auto* const pWindow = GetCurrentWindow();
    if (pWindow->SkipItems) {
        return 0.0f;
    }
    return pWindow->StateStorage.GetFloat(  //
        pWindow->GetID(ICB_PREFIX "ItemCurrentSize"));
}

IMCOOLBAR_API float ImGui::GetCoolBarItemScale() {
    auto* const pWindow = GetCurrentWindow();
    if (pWindow->SkipItems) {
        return 0.0f;
    }
    return pWindow->StateStorage.GetFloat(  //
        pWindow->GetID(ICB_PREFIX "ItemCurrentScale"));
}

IMCOOLBAR_API void ImGui::ShowCoolBarMetrics(bool* apoOpen) {
    if (ImGui::Begin("ImCoolBar Metrics", apoOpen)) {
        auto& g = *GImGui;
        for (auto* pWindow : g.Windows) {
            if (pWindow->StateStorage.Data.Size == 0) {
                continue;
            }
            const auto type = pWindow->StateStorage.GetInt(pWindow->GetID(ICB_PREFIX "Type"));
            if (type == ICB_TYPE_MAGIC) {
                if (!TreeNode(pWindow, "ImCoolBar %s", pWindow->Name)) {
                    continue;
                }

                const auto cbFlags = pWindow->StateStorage.GetInt(pWindow->GetID(ICB_PREFIX "Flags"));
                const auto anchor = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "Anchor"));
                const auto maxIdx = pWindow->StateStorage.GetInt(pWindow->GetID(ICB_PREFIX "ItemIdx"));
                const auto animScale = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "AnimScale"));
                const auto normalSize = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "NormalSize"));
                const auto hoveredSize = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "HoveredSize"));
                const auto effectStrength = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "EffectStrength"));
                const auto itemLastSize = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "ItemCurrentSize"));
                const auto itemLastScale = pWindow->StateStorage.GetFloat(pWindow->GetID(ICB_PREFIX "ItemCurrentScale"));

#define SetColumnLabel(a, fmt, v) \
    ImGui::TableNextColumn();     \
    ImGui::Text("%s", a);         \
    ImGui::TableNextColumn();     \
    ImGui::Text(fmt, v);          \
    ImGui::TableNextRow()

                if (ImGui::BeginTable("CoolbarDebugDatas", 2)) {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    SetColumnLabel("MaxIdx ", "%i", maxIdx);
                    SetColumnLabel("Anchor ", "%f", anchor);
                    SetColumnLabel("AnimScale ", "%f", animScale);
                    SetColumnLabel("NormalSize ", "%f", normalSize);
                    SetColumnLabel("HoveredSize ", "%f", hoveredSize);
                    SetColumnLabel("EffectStrength ", "%f", effectStrength);
                    SetColumnLabel("ItemLastSize ", "%f", itemLastSize);
                    SetColumnLabel("ItemLastScale ", "%f", itemLastScale);

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", "Flags ");
                    ImGui::TableNextColumn();
                    if (cbFlags == ImCoolBarFlags_None) {
                        ImGui::Text("None");
                    }
                    if (cbFlags & ImCoolBarFlags_Vertical) {
                        ImGui::Text("Vertical");
                    }
                    if (cbFlags & ImCoolBarFlags_Horizontal) {
                        ImGui::Text("Horizontal");
                    }
                    ImGui::TableNextRow();

                    for (int idx = 0; idx < maxIdx; ++idx) {
                        const auto coolbarItemId = pWindow->GetID(pWindow->ID + idx + 1);
                        const auto currentItemSize = pWindow->StateStorage.GetFloat(coolbarItemId);
                        ImGui::TableNextColumn();
                        ImGui::Text("Item %i Size ", idx);
                        ImGui::TableNextColumn();
                        ImGui::Text("%f", currentItemSize);
                        ImGui::TableNextRow();
                    }

                    ImGui::EndTable();
                }

#undef SetColumnLabel
                TreePop();
            }
        }
    }
    ImGui::End();
}

IMCOOLBAR_API bool ImGui::CoolBarDebugCheckVersion(const char* aVersion, size_t aConfigSize) {
    auto ok = true;
    if (strcmp(aVersion, IMCOOLBAR_VERSION) != 0) {
        ok = false;
        IM_ASSERT(false && "ImCoolBar version mismatch");
    }
    if (aConfigSize != sizeof(ImCoolBarConfig)) {
        ok = false;
        IM_ASSERT(false && "ImCoolBarConfig size mismatch");
    }
    return ok;
}

IMCOOLBAR_API void ImGui::ShowCoolBarDemoWindow(bool* apoOpen) {
    if (apoOpen != nullptr && !(*apoOpen)) {
        return;
    }
    if (!ImGui::Begin("ImCoolBar Demo", apoOpen)) {
        ImGui::End();
        return;
    }
    ImGui::Text("ImCoolBar %s", IMCOOLBAR_VERSION);
    ImGui::Separator();

    // Static demo state
    static ImCoolBarConfig s_config;
    static ImCoolBarFlags s_flags = ImCoolBarFlags_Horizontal;
    static int32_t s_itemCount = 5;
    static bool s_showMetrics = false;

    if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto orient = (s_flags & ImCoolBarFlags_Horizontal) ? 0 : 1;
        if (ImGui::Combo("Orientation", &orient, "Horizontal\0Vertical\0\0")) {
            s_flags = (orient == 0) ? ImCoolBarFlags_Horizontal : ImCoolBarFlags_Vertical;
        }

        ImGui::SliderFloat("Normal Size", &s_config.normalSize, 10.0f, 200.0f, "%.0f");
        ImGui::SliderFloat("Hovered Size", &s_config.hoveredSize, s_config.normalSize, 400.0f, "%.0f");
        ImGui::SliderFloat("Anim Step", &s_config.animStep, 0.001f, 0.5f, "%.3f");
        ImGui::SliderFloat("Effect Strength", &s_config.effectStrength, 0.1f, 2.0f, "%.2f");
        ImGui::SliderFloat("Anchor X", &s_config.anchor.x, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Anchor Y", &s_config.anchor.y, -1.0f, 1.0f, "%.2f");
        ImGui::SliderInt("Item Count", &s_itemCount, 1, 20);
        ImGui::Separator();
        if (ImGui::Button("Reset Defaults")) {
            s_config = ImCoolBarConfig();
            s_flags = ImCoolBarFlags_Horizontal;
            s_itemCount = 5;
        }
    }

    if (ImGui::CollapsingHeader("Preview", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped("The CoolBar is rendered as a top-level window (see below the demo window).");
        ImGui::Checkbox("Show Metrics", &s_showMetrics);
    }

    ImGui::End();

    // Draw the demo CoolBar
    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::BeginCoolBar("##DemoCoolBar", s_flags, s_config)) {
        for (int32_t i = 0; i < s_itemCount; ++i) {
            if (ImGui::CoolBarItem()) {
                const auto w = ImGui::GetCoolBarItemWidth();
                ImGui::PushID(i);
                char label[16];
                ImFormatString(label, sizeof(label), "%d", i + 1);
                ImGui::Button(label, ImVec2(w, w));
                ImGui::PopID();
            }
        }
        ImGui::EndCoolBar();
    }

    if (s_showMetrics) {
        ImGui::ShowCoolBarMetrics(&s_showMetrics);
    }
}

/////////////////////////////
// C API
/////////////////////////////

IMCOOLBAR_C_API const char* ImCoolBar_GetVersion(void) {
    return IMCOOLBAR_VERSION;
}

IMCOOLBAR_C_API int ImCoolBar_GetVersionNum(void) {
    return IMCOOLBAR_VERSION_NUM;
}

IMCOOLBAR_C_API bool ImCoolBar_DebugCheckVersion(const char* aVersion, size_t aConfigSize) {
    return ImGui::CoolBarDebugCheckVersion(aVersion, aConfigSize);
}

IMCOOLBAR_C_API bool ImCoolBar_BeginCoolBar(const char* aLabel,
                                            int aCBFlags,
                                            float aNormalSize,
                                            float aHoveredSize,
                                            float aAnimStep,
                                            float aEffectStrength,
                                            float aAnchorX,
                                            float aAnchorY,
                                            int aWinFlags) {
    ImGui::ImCoolBarConfig config(ImVec2(aAnchorX, aAnchorY), aNormalSize, aHoveredSize, aAnimStep, aEffectStrength);
    return ImGui::BeginCoolBar(aLabel, aCBFlags, config, aWinFlags);
}

IMCOOLBAR_C_API void ImCoolBar_EndCoolBar(void) {
    ImGui::EndCoolBar();
}

IMCOOLBAR_C_API bool ImCoolBar_CoolBarItem(void) {
    return ImGui::CoolBarItem();
}

IMCOOLBAR_C_API float ImCoolBar_GetCoolBarItemWidth(void) {
    return ImGui::GetCoolBarItemWidth();
}

IMCOOLBAR_C_API float ImCoolBar_GetCoolBarItemScale(void) {
    return ImGui::GetCoolBarItemScale();
}

IMCOOLBAR_C_API void ImCoolBar_ShowCoolBarMetrics(bool* apoOpen) {
    ImGui::ShowCoolBarMetrics(apoOpen);
}

IMCOOLBAR_C_API void ImCoolBar_ShowCoolBarDemoWindow(bool* apoOpen) {
    ImGui::ShowCoolBarDemoWindow(apoOpen);
}
