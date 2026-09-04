#include "MorphImGui.h"
#include "MorphEditor.h"
#include "MorphProject.h"
#include "MorphPlatform.h"
#include "MorphTypes.h"
#include "imgui.h"
#include <cstdio>
#include <cstring>

typedef enum
{
    HUB_MAIN,
    HUB_CREATE,
    HUB_OPEN,
} HubMode;

static HubMode hub = HUB_MAIN;

extern "C"
{

void morphImGuiDrawHub(MorphEditor* editor, MorphCamera* camera)
{
    ImGui::OpenPopup("##hub");
    ImGui::SetNextWindowSizeConstraints(ImVec2(550.0f, 0.0f), ImVec2(550.0f, FLT_MAX));
    if (ImGui::BeginPopupModal("##hub", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Morph engine");
        ImGui::TextDisabled("Recent projects");
        ImGui::Separator();

        int cols = 5;

        if (hub == HUB_MAIN)
        {
            if (ImGui::BeginTable("RecentGrid", cols))
            {
                for (int i = 0; i < MAX_RECENT; i++)
                {
                    ImGui::TableNextColumn();
                    ImGui::PushID(i);

                    if (i < (int)editor->recentCount)
                    {
                        const char* name = strrchr(editor->recentProjects[i], '\\');
                        name = name ? name + 1 : editor->recentProjects[i];

                        if (ImGui::Button(name, ImVec2(100, 100)))
                            morphEditorOpenProject(editor, camera, editor->recentProjects[i]);
                    }
                    else
                    {
                        ImGui::BeginDisabled();
                        ImGui::Button("empty", ImVec2(100, 100));
                        ImGui::EndDisabled();
                    }

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }

            ImGui::Separator();
            if (ImGui::Button("New Project"))  { hub = HUB_CREATE; }
            ImGui::SameLine();
            if (ImGui::Button("Open Project")) { hub = HUB_OPEN; }
        }
        else if (hub == HUB_CREATE)
        {
            static char projectName[128] = "MyGame";
            static char projectLocation[MAX_PATH_LEN] = "C:\\Users\\";

            ImGui::InputText("Name", projectName, sizeof(projectName));
            ImGui::InputText("Location", projectLocation, sizeof(projectLocation));
            ImGui::SameLine();
            if (ImGui::Button("..."))
            {
                char folder[MAX_PATH_LEN] = {0};
                if (morphPlatformOpenFolderDialog(folder, MAX_PATH_LEN))
                    strncpy(projectLocation, folder, sizeof(projectLocation));
            }

            if (ImGui::Button("Create"))
            {
                char mproj[MAX_PATH_LEN];
                if (morphProjectCreate(&editor->project, projectName, projectLocation, editor->exeDir))
                {
                    snprintf(mproj, sizeof(mproj), "%s\\%s\\%s.mproj", projectLocation, projectName, projectName);
                    morphEditorOpenProject(editor, camera, mproj);
                }
                else
                {
                    morphProjectCreate(&editor->project, "Untitled", editor->exeDir, editor->exeDir);
                    editor->project.temporary = true;
                    editor->showHUB = true;

                    snprintf(editor->imguiIniPath, MAX_PATH_LEN, "%s\\Untitled\\Engine\\imgui.ini", editor->exeDir);
                    morphImGuiSetIniPath(editor->imguiIniPath);
                }
                hub = HUB_MAIN;
            }
            if (ImGui::Button("Cancel")) { hub = HUB_MAIN; }
        }
        else if (hub == HUB_OPEN)
        {
            char path[MAX_PATH_LEN] = {0};
            if (morphPlatformOpenFileDialog(path, MAX_PATH_LEN, "Morph project\0*.mproj\0"))
                morphEditorOpenProject(editor, camera, path);
            hub = HUB_MAIN;
        }

        ImGui::EndPopup();
    }
}

}