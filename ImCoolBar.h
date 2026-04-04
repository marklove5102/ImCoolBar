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

#pragma once

// ImCoolBar, v1.0.0
// macOS Dock-style magnification bar for Dear ImGui

#define IMCOOLBAR_VERSION "1.0.0"
#define IMCOOLBAR_VERSION_NUM 10000

#ifndef IMCOOLBAR_API
#define IMCOOLBAR_API
#endif

#include <stdbool.h>
#include <stddef.h>

typedef int ImCoolBarFlags;                //
enum ImCoolBarFlags_ {                     //
    ImCoolBarFlags_None = 0,               //
    ImCoolBarFlags_Vertical = (1 << 0),    //
    ImCoolBarFlags_Horizontal = (1 << 1),  //
};

#ifdef __cplusplus

#include "imgui.h"

// #define ENABLE_IMCOOLBAR_DEBUG

namespace ImGui {

struct ImCoolBarConfig {
    ImVec2 anchor{-1.0f, -1.0f};
    float normalSize{40.0f};
    float hoveredSize{60.0f};
    float animStep{0.15f};
    float effectStrength{0.5f};
    ImCoolBarConfig(const ImVec2 aAnchor = ImVec2(-1.0f, -1.0f),
                    const float& aNormalSize = 40.0f,
                    const float& aHoveredSize = 60.0f,
                    const float& aAnimStep = 0.15f,
                    const float& aEffectStrength = 0.5f)
        : anchor(aAnchor), normalSize(aNormalSize), hoveredSize(aHoveredSize), animStep(aAnimStep), effectStrength(aEffectStrength) {
    }
};

// Check that version and struct sizes match between compiled ImCoolBar code and caller.
#define IMCOOLBAR_CHECKVERSION() ImGui::CoolBarDebugCheckVersion(IMCOOLBAR_VERSION, sizeof(ImGui::ImCoolBarConfig))

IMCOOLBAR_API bool CoolBarDebugCheckVersion(const char* aVersion, size_t aConfigSize);
IMCOOLBAR_API bool BeginCoolBar(const char* aLabel,
                                ImCoolBarFlags aCBFlags = ImCoolBarFlags_Vertical,
                                const ImCoolBarConfig& arConfig = {},
                                ImGuiWindowFlags aWinFlags = ImGuiWindowFlags_None);
IMCOOLBAR_API void EndCoolBar();
IMCOOLBAR_API bool CoolBarItem();
IMCOOLBAR_API float GetCoolBarItemWidth();
IMCOOLBAR_API float GetCoolBarItemScale();
IMCOOLBAR_API void ShowCoolBarMetrics(bool* apoOpen);
IMCOOLBAR_API void ShowCoolBarDemoWindow(bool* apoOpen = nullptr);

}  // namespace ImGui

#endif  // __cplusplus

/////////////////////////////////////////////////
////// C LANG API ///////////////////////////////
/////////////////////////////////////////////////

#ifdef __cplusplus
#define IMCOOLBAR_C_API extern "C" IMCOOLBAR_API
#else  // __cplusplus
#define IMCOOLBAR_C_API
#endif  // __cplusplus

IMCOOLBAR_C_API const char* ImCoolBar_GetVersion(void);
IMCOOLBAR_C_API int ImCoolBar_GetVersionNum(void);
IMCOOLBAR_C_API bool ImCoolBar_DebugCheckVersion(const char* aVersion, size_t aConfigSize);
IMCOOLBAR_C_API bool ImCoolBar_BeginCoolBar(const char* aLabel,
                                            int aCBFlags,
                                            float aNormalSize,
                                            float aHoveredSize,
                                            float aAnimStep,
                                            float aEffectStrength,
                                            float aAnchorX,
                                            float aAnchorY,
                                            int aWinFlags);
IMCOOLBAR_C_API void ImCoolBar_EndCoolBar(void);
IMCOOLBAR_C_API bool ImCoolBar_CoolBarItem(void);
IMCOOLBAR_C_API float ImCoolBar_GetCoolBarItemWidth(void);
IMCOOLBAR_C_API float ImCoolBar_GetCoolBarItemScale(void);
IMCOOLBAR_C_API void ImCoolBar_ShowCoolBarMetrics(bool* apoOpen);
IMCOOLBAR_C_API void ImCoolBar_ShowCoolBarDemoWindow(bool* apoOpen);
