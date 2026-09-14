#include <imgui.h>
#include <imgui_internal.h>
#include "MorphImGui.h"
#include "MorphEditor.h"
#include "MorphMath.h"
#include "MorphTypes.h"
#include "GLFW/glfw3.h"
#include <cstdio>

static void handleWindowDrag(GLFWwindow* window)
{
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        int windowPosX, windowPosY;
        glfwGetWindowPos(window, &windowPosX, &windowPosY);
        glfwSetWindowPos(window, windowPosX + (int)delta.x, windowPosY + (int)delta.y);
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
    }
}

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

    ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;

    f32 fontSize = ImGui::GetFontSize();
    f32 rowHeight = fontSize + ImGui::GetStyle().FramePadding.y * 6.0f;
    f32 barHeight = rowHeight * 2.0f;
    f32 screenWidth = ImGui::GetIO().DisplaySize.x;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 25.0f);
    morphImGuiBeginDockspace(barHeight);
    ImGui::PopStyleVar();

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize        |
        ImGuiWindowFlags_NoDecoration    |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.048f, 0.048f, 0.048f, 1.0f));
    
    //left block ----------
    ImGui::SetNextWindowPos(viewportPos);
    ImGui::SetNextWindowSize(ImVec2(rowHeight, barHeight), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("##leftblock", nullptr, flags);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 logoCenter(windowPos.x + rowHeight * 0.5f, windowPos.y + barHeight * 0.5f);
        f32 logoFontSize = 27.0f;

        drawList->PushClipRectFullScreen();
        morphAddTextVertical(drawList, "Morph", logoCenter, ImGui::GetColorU32(ImGuiCol_Text), logoFontSize);
        drawList->PopClipRect();

        handleWindowDrag(window);
    ImGui::End();

    //right block --------
    char fpsText[32];
    snprintf(fpsText, sizeof(fpsText), "%.1f FPS  (%.2f ms)", 1.0f / deltaTime, deltaTime * 1000.0f);

    f32 btnWidth = 40.0f;
    f32 controlsWidth = btnWidth * 3.0f;
    f32 statsWidth = ImGui::CalcTextSize("9999.9 FPS  (99.99 ms)").x;
    f32 rightBlockWidth = statsWidth + controlsWidth + 40.0f;

    bool isMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);

    float islandPad = barHeight * 0.08f;
    ImGui::SetNextWindowPos(ImVec2(viewportPos.x + screenWidth - rightBlockWidth, viewportPos.y + islandPad));
    ImGui::SetNextWindowSize(ImVec2(rightBlockWidth, barHeight - islandPad * 2.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.013725487f, 0.013725487f, 0.013725487f, 1.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));

    ImGui::Begin("##rightblock", nullptr, flags);

        float contentHeight = ImGui::GetFrameHeight() * 2.0f + ImGui::GetStyle().ItemSpacing.y;
        float windowHeight = barHeight - islandPad * 2.0f;
        ImGui::SetCursorPos(ImVec2(15.0f, (windowHeight - contentHeight) * 0.5f));

        ImGui::Text("%s", fpsText);
        ImGui::SetCursorPos(ImVec2(statsWidth * 1.12f, (windowHeight - contentHeight) * 0.5f));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.035f, 0.035f, 0.035f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.035f, 0.035f, 0.035f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.035f, 0.035f, 0.035f, 1.0f));

        if (ImGui::Button("_", ImVec2(btnWidth, fontSize + 5.0f)))
            glfwIconifyWindow(window);
        ImGui::SameLine();

        if (ImGui::Button(isMaximized ? "[ ]" : "[   ]", ImVec2(btnWidth, fontSize + 5.0f)))
        {
            if (isMaximized) glfwRestoreWindow(window);
            else glfwMaximizeWindow(window);
        }
        ImGui::SameLine();

        if (ImGui::Button("X", ImVec2(btnWidth, fontSize + 5.0f)))
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        
        ImGui::SetCursorPos(ImVec2(15.0f, fontSize * 1.35f));
        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("%s", editor->project.name);
        ImGui::SetWindowFontScale(1.0f);

        handleWindowDrag(window);
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    
    //iland curves
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 wMin = ImVec2(viewportPos.x + screenWidth - rightBlockWidth, viewportPos.y + islandPad);
    ImVec2 wMax = ImVec2(viewportPos.x + screenWidth, viewportPos.y + barHeight - islandPad);
    
    ImU32 islandCol = IM_COL32(3, 3, 3, 255);
    dl->AddRectFilled(wMin, wMax, islandCol, 15.0f, ImDrawFlags_RoundCornersLeft);

    float cr = islandPad * 1.5f;
    float shift = -1.0f;
    ImU32 barBg = IM_COL32(12, 12, 12, 255);

    //top right
    dl->AddRectFilled(ImVec2(wMax.x - cr, wMin.y - cr), ImVec2(wMax.x, wMin.y), islandCol);
    dl->PathArcTo(ImVec2(wMax.x - cr, wMin.y - cr), cr, 0.0f, MORPH_PI * 0.5f);
    dl->PathLineTo(ImVec2(wMax.x - cr, wMin.y - cr));
    dl->PathFillConvex(barBg);

    //bottom right
    dl->AddRectFilled(ImVec2(wMax.x - cr - shift, wMax.y), ImVec2(wMax.x - shift, wMax.y + cr), islandCol);
    dl->PathArcTo(ImVec2(wMax.x - cr - shift, wMax.y + cr), cr, MORPH_PI * 1.5f, MORPH_PI * 2.0f);
    dl->PathLineTo(ImVec2(wMax.x - cr - shift, wMax.y + cr));
    dl->PathFillConvex(barBg);


    //center block -----------
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0));

    ImGui::SetNextWindowPos(ImVec2(viewportPos.x + rowHeight, viewportPos.y));
    ImGui::SetNextWindowSize(ImVec2(screenWidth - rowHeight - rightBlockWidth, rowHeight), ImGuiCond_Always);
    ImGui::Begin("##centerblockMENU", nullptr, flags);
        ImGui::SameLine(0.0f, 15.0f);
        float btnHeight = ImGui::GetFrameHeight();
        float offsetY = (rowHeight * 0.25f);
        ImGui::SetCursorPosY(offsetY);

        if (ImGui::Button("File")) ImGui::OpenPopup("##filemenu");
        if (ImGui::BeginPopup("##filemenu"))
        {
            ImGui::MenuItem("Open HUB", NULL, &editor->showHUB);
            ImGui::MenuItem("Import", NULL, nullptr);
            ImGui::EndPopup();
        }
        ImGui::SameLine(0.0f, 25.0f);
        ImGui::SetCursorPosY(offsetY);
        if (ImGui::Button("Edit")) ImGui::OpenPopup("##editmenu");
        if (ImGui::BeginPopup("##editmenu"))
        {
            ImGui::MenuItem("Project settings", NULL, nullptr);
            ImGui::EndPopup();
        }
        ImGui::SameLine(0.0f, 25.0f);
        ImGui::SetCursorPosY(offsetY);
        if (ImGui::Button("Window")) ImGui::OpenPopup("##windowmenu");
        if (ImGui::BeginPopup("##windowmenu"))
        {
            ImGui::MenuItem("Output", NULL, &editor->showOutput);
            ImGui::MenuItem("Content drawer", NULL, &editor->showContentDrawer);
            ImGui::MenuItem("Tools", NULL, &editor->showTools);
            ImGui::MenuItem("Outliner", NULL, &editor->showOutliner);
            ImGui::MenuItem("Details", NULL, &editor->showDetails);
            ImGui::MenuItem("Viewport", NULL, &editor->showViewport);
            ImGui::EndPopup();
        }
        ImGui::SameLine(0.0f, 25.0f);
        ImGui::SetCursorPosY(offsetY);
        if (ImGui::Button("Build")) ImGui::OpenPopup("##buildmenu");
        if (ImGui::BeginPopup("##buildmenu"))
        {
            ImGui::MenuItem("Build preferences", NULL, nullptr);
            ImGui::MenuItem("Build", NULL, nullptr);
            ImGui::EndPopup();
        }

        handleWindowDrag(window);
    ImGui::End();
    ImGui::PopStyleColor(3);

    ImGui::SetNextWindowPos(ImVec2(viewportPos.x + rowHeight, viewportPos.y + rowHeight));
    ImGui::SetNextWindowSize(ImVec2(screenWidth - rowHeight - rightBlockWidth, rowHeight), ImGuiCond_Always);

    ImGui::Begin("##centerblockTABS", nullptr, flags);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(40.0f, (rowHeight - fontSize) * 0.4f));
        ImGui::PushStyleVar(ImGuiStyleVar_TabBarBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 10.0f);
        float tabHeight = fontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
        ImGui::SetCursorPosY(rowHeight - tabHeight);
        if (ImGui::BeginTabBar("##tabs"))
        {
            ImVec2 activeTabMin(0,0), activeTabMax(0,0);
            ImDrawList* dl = ImGui::GetWindowDrawList();

            for (u32 i = 0; i < editor->tabCount; i++)
            {
                MorphEditorTab* tab = &editor->tabs[i];
                if (!tab->open) continue;

                char label[128];
                snprintf(label, sizeof(label), "%s%s###tab_%u", tab->title, tab->dirty ? "*" : "", i);

                bool keepOpen = true;
                ImGuiTabItemFlags flags = tab->dirty ? ImGuiTabItemFlags_UnsavedDocument : 0;

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0,0,0,0));

                if (ImGui::BeginTabItem(label, editor->tabCount > 1 ? &keepOpen : NULL, flags))
                {
                    editor->activeTab = i;
                    ImGui::EndTabItem();
                    activeTabMin = ImGui::GetItemRectMin();
                    activeTabMax = ImGui::GetItemRectMax();

                    ImVec2 textSize = ImGui::CalcTextSize(tab->title);
                    float iconSize = fontSize * 0.8f;
                    float totalWidth = iconSize + 8.0f + textSize.x;
                    float startX = (activeTabMin.x + activeTabMax.x) * 0.5f - totalWidth * 0.5f;
                    float iconX = startX;
                    float textX = startX + iconSize + 8.0f;
                    float textY = activeTabMin.y + (activeTabMax.y - activeTabMin.y) * 0.5f - textSize.y * 0.5f + 5.0f;
                    float iconY = textY + (textSize.y - iconSize) * 0.5f;

                    dl->AddRect(ImVec2(iconX, iconY), ImVec2(iconX + iconSize, iconY + iconSize), IM_COL32(255, 255, 255, 255));
                    dl->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), tab->title);
                }

                ImGui::PopStyleColor();
                if (!keepOpen) tab->open = false;
            }
            ImGui::EndTabBar();
            
            //selected chip curves
            float r = 10.0f;
            ImU32 barCol = IM_COL32(3, 3, 3, 255);
            ImU32 canvasCol = IM_COL32(12, 12, 12, 255);

            dl->PushClipRectFullScreen();

            //bottom left
            dl->AddRectFilled(ImVec2(activeTabMin.x - r, activeTabMax.y - r), ImVec2(activeTabMin.x, activeTabMax.y), barCol);
            dl->PathArcTo(ImVec2(activeTabMin.x - r, activeTabMax.y - r), r, 0.0f, MORPH_PI * 0.5f);
            dl->PathLineTo(ImVec2(activeTabMin.x - r, activeTabMax.y - r));
            dl->PathFillConvex(canvasCol);

            //bottom right
            dl->AddRectFilled( ImVec2(activeTabMax.x, activeTabMax.y - r), ImVec2(activeTabMax.x + r, activeTabMax.y), barCol);
            dl->PathArcTo(ImVec2(activeTabMax.x + r, activeTabMax.y - r), r, MORPH_PI * 0.5f, MORPH_PI);
            dl->PathLineTo(ImVec2(activeTabMax.x + r, activeTabMax.y - r));
            dl->PathFillConvex(canvasCol);

            dl->PopClipRect();
        }
        ImGui::PopStyleVar(3);

        handleWindowDrag(window);
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

}