#include "MorphImGui.h"
#include "MorphEditor.h"
#include "MorphMath.h"
#include "MorphTypes.h"
#include "imgui.h"
#include "GLFW/glfw3.h"
#include <cstdio>

static void morphAddTextVertical(ImDrawList* drawList, const char* text, ImVec2 center, ImU32 color, f32 fontSize, f32 angle = -MORPH_PI * 0.5f)
{
    ImFont* font = ImGui::GetFont();
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    ImVec2 drawPos(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f);

    int vtxStart = drawList->VtxBuffer.Size;
    drawList->AddText(font, fontSize, drawPos, color, text);
    int vtxEnd = drawList->VtxBuffer.Size;

    for (int i = vtxStart; i < vtxEnd; i++)
    {
        ImDrawVert& vtx = drawList->VtxBuffer[i];
        Vec2 local = vec2Rotate(vec2(vtx.pos.x - center.x, vtx.pos.y - center.y), angle);
        vtx.pos.x = center.x + local.x;
        vtx.pos.y = center.y + local.y;
    }
}

extern "C"
{

void morphImGuiDrawMenuBar(MorphEditor* editor, GLFWwindow* window, f32 deltaTime)
{
    f32 fontSize = ImGui::GetFontSize();
    f32 rowHeight = fontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
    f32 totalHeight = rowHeight * 3.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, (totalHeight - fontSize) * 0.5f));
    bool mainMenuBarOpen = ImGui::BeginMainMenuBar();
    ImGui::PopStyleVar();

    if (mainMenuBarOpen)
    {
        //left block
        f32 logoFontSize = 20.0f;
        ImFont* font = ImGui::GetFont();
        ImVec2 textSize = font->CalcTextSizeA(logoFontSize, FLT_MAX, 0.0f, "Morph");

        if (totalHeight < textSize.x + 16.0f)
            totalHeight = textSize.x + 16.0f;

        f32 leftBlockWidth = textSize.y + 16.0f;

        ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
        ImGui::PushID("EngineLogo");
        bool logoClicked = ImGui::InvisibleButton("##logo", ImVec2(leftBlockWidth, totalHeight));
        bool logoHovered = ImGui::IsItemHovered();
        ImGui::PopID();

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 logoMin = ImGui::GetItemRectMin();
        ImVec2 logoMax = ImGui::GetItemRectMax();
        ImVec2 logoCenter((logoMin.x + logoMax.x) * 0.5f, (logoMin.y + logoMax.y) * 0.5f);

        if (logoHovered)
            drawList->AddRectFilled(logoMin, logoMax, ImGui::GetColorU32(ImGuiCol_ButtonHovered));

        morphAddTextVertical(drawList, "Morph", logoCenter, ImGui::GetColorU32(ImGuiCol_Text), logoFontSize);

        if (logoClicked)
            editor->showHUB = true;

        //center block menu row
        f32 centerX = leftBlockWidth + 4.0f;
        ImGui::SetCursorPos(ImVec2(centerX, -rowHeight * 0.5f));
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("Open HUB", NULL, &editor->showHUB);
            ImGui::MenuItem("Import", NULL, nullptr);
            ImGui::EndMenu();
        }
        ImGui::SetCursorPosY(-rowHeight * 0.5f);
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::MenuItem("Project settings", NULL, nullptr);
            ImGui::EndMenu();
        }
        ImGui::SetCursorPosY(-rowHeight * 0.5f);
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Output", NULL, &editor->showOutput);
            ImGui::MenuItem("Content drawer", NULL, &editor->showContentDrawer);
            ImGui::MenuItem("Tools", NULL, &editor->showTools);
            ImGui::MenuItem("Outliner", NULL, &editor->showOutliner);
            ImGui::MenuItem("Details", NULL, &editor->showDetails);
            ImGui::MenuItem("Viewport", NULL, &editor->showViewport);
            ImGui::EndMenu();
        }
        ImGui::SetCursorPosY(-rowHeight * 0.5f);
        if (ImGui::BeginMenu("Build"))
        {
            ImGui::MenuItem("Build preferences", NULL, nullptr);
            ImGui::MenuItem("Build", NULL, nullptr);
            ImGui::EndMenu();
        }

        //center block tabs row
        ImGui::SetCursorPos(ImVec2(centerX, rowHeight * 1.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, (rowHeight * 1.5f - fontSize) * 0.5f));
        if (ImGui::BeginTabBar("##tabs"))
        {
            for (u32 i = 0; i < editor->tabCount; i++)
            {
                MorphEditorTab* tab = &editor->tabs[i];
                if (!tab->open) continue;

                char label[128];
                snprintf(label, sizeof(label), "%s%s###tab_%u", tab->title, tab->dirty ? "*" : "", i);

                bool keepOpen = true;
                ImGuiTabItemFlags flags = tab->dirty ? ImGuiTabItemFlags_UnsavedDocument : 0;

                if (ImGui::BeginTabItem(label, editor->tabCount > 1 ? &keepOpen : NULL, flags))
                {
                    editor->activeTab = i;
                    ImGui::EndTabItem();
                }
                if (!keepOpen) tab->open = false;
            }
            ImGui::EndTabBar();
        }
        ImGui::PopStyleVar();

        //right block
        char fpsText[32];
        snprintf(fpsText, sizeof(fpsText), "%.1f FPS  (%.2f ms)", 1.0f / deltaTime, deltaTime * 1000.0f);

        f32 btnWidth       = 40.0f;
        f32 controlsWidth  = btnWidth * 3.0f;
        f32 statsWidth     = ImGui::CalcTextSize(fpsText).x;
        f32 rightPanelWidth = statsWidth + controlsWidth + 30.0f;

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - rightPanelWidth, -rowHeight * 0.5f));
        ImGui::Text("%s", fpsText);
        ImGui::SameLine();

        ImGui::SetCursorPosY(0.0f);
        if (ImGui::Button("_", ImVec2(btnWidth, rowHeight)))
            glfwIconifyWindow(window);
        ImGui::SameLine();

        ImGui::SetCursorPosY(0.0f);
        bool isMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
        if (ImGui::Button(isMaximized ? "[]" : "[ ]", ImVec2(btnWidth, rowHeight)))
        {
            if (isMaximized) glfwRestoreWindow(window);
            else             glfwMaximizeWindow(window);
        }
        ImGui::SameLine();

        ImGui::SetCursorPosY(0.0f);
        if (ImGui::Button("X", ImVec2(btnWidth, rowHeight)))
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - rightPanelWidth, (rowHeight - fontSize)));
        ImGui::Text("%s", editor->project.name);

        //dragging window
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() &&
            ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            int wx, wy;
            glfwGetWindowPos(window, &wx, &wy);
            glfwSetWindowPos(window, wx + (int)delta.x, wy + (int)delta.y);
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }

        ImGui::EndMainMenuBar();
    }
}

}