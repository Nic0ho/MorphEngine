#include "MorphImGui.h"
#include "MorphAssetType.h"
#include "MorphEditor.h"
#include "MorphLog.h"
#include "MorphProject.h"
#include "MorphTypes.h"
#include "MorphPlatform.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>

#include <windows.h>

struct FileItem
{
    char name[256];
    bool isDir;
};

static char selectedPath[MAX_PATH_LEN] = "";
static char pathHistory[MAX_HISTORY][MAX_PATH_LEN];
static int pathHistoryCount = 0;
static int pathHistoryIndex = -1;

extern "C"
{
bool morphImGuiInit(MorphVulkanContext *ctx, GLFWwindow *window)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1000;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    vkCreateDescriptorPool(ctx->logicalDevice, &poolInfo, nullptr, &ctx->imguiDescriptorPool);

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplGlfw_InitForVulkan(window, true);

    VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo = {};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipelineRenderingInfo.colorAttachmentCount = 1;
    pipelineRenderingInfo.pColorAttachmentFormats = &ctx->swapchainFormat;

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.ApiVersion         = VK_API_VERSION_1_3;
    initInfo.Instance           = ctx->instance;
    initInfo.PhysicalDevice     = ctx->physicalDevice;
    initInfo.Device             = ctx->logicalDevice;
    initInfo.QueueFamily        = ctx->graphicsFamily;
    initInfo.Queue              = ctx->graphicsQueue;
    initInfo.DescriptorPoolSize = 1000;
    initInfo.MinImageCount      = 2;
    initInfo.ImageCount         = ctx->swapchainImageCount;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;

    ImGui_ImplVulkan_Init(&initInfo);

    return true;
}

void morphImGuiNewFrame(void)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void morphImGuiEndFrame(void)
{
    ImGui::EndFrame();
    ImGui::UpdatePlatformWindows();
}

void morphImGuiRender(VkCommandBuffer cmd)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

void morphImGuiShutdown(MorphVulkanContext* ctx)
{
    vkDeviceWaitIdle(ctx->logicalDevice);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(ctx->logicalDevice, ctx->imguiDescriptorPool, nullptr);
}

void morphImGuiBeginDockspace(void)
{ ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode); }

void morphImGuiBeginWindow(const char* name)
{ ImGui::Begin(name); }

void morphImGuiEndWindow(void)
{ ImGui::End(); }

VkDescriptorSet morphImGuiRegisterTexture(VkSampler sampler, VkImageView view)
{ return ImGui_ImplVulkan_AddTexture(sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); }

void morphImGuiDrawOutput(MorphOutputConsoleBuffer* buffer)
{
    ImGui::Separator();

    f32 footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

    ImGui::BeginChild("OutlinerList", ImVec2(0, -footerHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

    bool full = buffer->count == MAX_CONSOLE_OUTPUT_LINES;
    u32 start = full ? buffer->writeIndex : 0;

    for (u32 i = 0; i < buffer->count; i++)
    {
        u32 index = (start + i) % MAX_CONSOLE_OUTPUT_LINES;
        ImGui::TextUnformatted(buffer->messages[index]);
    }

    ImGui::EndChild();
}

void morphImGuiResetContentBrowser(void)
{ selectedPath[0] = '\0'; }

static void navigateTo(const char* newPath, bool recordHistory)
{
    snprintf(selectedPath, sizeof(selectedPath), "%s", newPath);

    if (recordHistory)
    {
        if (pathHistoryIndex < pathHistoryCount - 1)
            pathHistoryCount = pathHistoryIndex + 1;

        if (pathHistoryCount < MAX_HISTORY)
        {
            snprintf(pathHistory[pathHistoryCount], MAX_PATH_LEN, "%s", newPath);
            pathHistoryCount++;
            pathHistoryIndex++;
        }
        else
        {
            for (int i = 1; i < MAX_HISTORY; i++)
                snprintf(pathHistory[i - 1], MAX_PATH_LEN, "%s", pathHistory[i]);
            snprintf(pathHistory[MAX_HISTORY - 1], MAX_PATH_LEN, "%s", newPath);
        }
    }
}

static bool hasSubdirectories(const char* path)
{
    WIN32_FIND_DATAA findData;
    char searchPath[MAX_PATH_LEN];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", path);

    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return false;

    do
    {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            FindClose(hFind);
            return true;
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    return false;
}

static void drawFileTree(const char* path)
{
    WIN32_FIND_DATAA findData;
    char searchPath[MAX_PATH_LEN];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", path);

    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do
    {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;

        char childPath[MAX_PATH_LEN];
        snprintf(childPath, sizeof(childPath), "%s\\%s", path, findData.cFileName);

        bool selected = strcmp(selectedPath, childPath) == 0;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (selected) flags |= ImGuiTreeNodeFlags_Selected;

        if (!hasSubdirectories(childPath)) flags |= ImGuiTreeNodeFlags_Leaf;

        bool open = ImGui::TreeNodeEx(findData.cFileName, flags);
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            navigateTo(childPath, true);

        if (open)
        {
            drawFileTree(childPath);
            ImGui::TreePop();
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
}

void morphImGuiDrawAssetBrowser(MorphEditor* editor)
{
    ImGui::Separator();

    if (ImGui::TreeNodeEx("Project", ImGuiTreeNodeFlags_DefaultOpen))
    {
        char projectPath[MAX_PATH_LEN];
        snprintf(projectPath, sizeof(projectPath), "%s\\Project", editor->project.rootPath);
        drawFileTree(projectPath);
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Engine", NULL))
    {
        char enginePath[MAX_PATH_LEN];
        snprintf(enginePath, sizeof(enginePath), "%s\\Engine", editor->project.rootPath);
        drawFileTree(enginePath);
        ImGui::TreePop();
    }
}

static int compareFileItems(const void* a, const void* b)
{
    const FileItem* itemA = (const FileItem*)a;
    const FileItem* itemB = (const FileItem*)b;

    if (itemA->isDir != itemB->isDir)
        return itemB->isDir ? 1 : -1;

    return strcmp(itemA->name, itemB->name);
}

static void drawFolderContents(MorphEditor* editor)
{
    //top bar
    if (selectedPath[0] == '\0')
        navigateTo(editor->project.rootPath, true);

    bool canGoBack = pathHistoryIndex > 0;
    bool canGoForward = pathHistoryIndex < pathHistoryCount - 1;

    if (!canGoBack) ImGui::BeginDisabled();
    if (ImGui::Button("<"))
    {
        pathHistoryIndex--;
        navigateTo(pathHistory[pathHistoryIndex], false);
    }
    if (!canGoBack) ImGui::EndDisabled();

    ImGui::SameLine();

    if (!canGoForward) ImGui::BeginDisabled();
    if (ImGui::Button(">"))
    {
        pathHistoryIndex++;
        navigateTo(pathHistory[pathHistoryIndex], false);
    }
    if (!canGoForward) ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::Text("%s", selectedPath);

    WIN32_FIND_DATAA findData;
    char searchPath[MAX_PATH_LEN];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", selectedPath);

    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    static FileItem items[1024]; 
    int itemCount = 0;

    do
    {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;
        const char* ext = strrchr(findData.cFileName, '.');
        if (ext && strcmp(ext, ".mproj") == 0) continue;
        
        if (itemCount >= 1024) break; 

        snprintf(items[itemCount].name, sizeof(items[itemCount].name), "%s", findData.cFileName);
        items[itemCount].isDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        itemCount++;
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);

    //content sort
    qsort(items, itemCount, sizeof(FileItem), compareFileItems);

    float padding = 16.0f;
    float iconSize = 64.0f; 
    float cellSize = iconSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    if (ImGui::BeginTable("ContentBrowserGrid", columnCount))
    {
        for (int i = 0; i < itemCount; i++)
        {
            ImGui::TableNextColumn();
            ImGui::PushID(i);

            
            char fullPath[MAX_PATH_LEN];
            snprintf(fullPath, sizeof(fullPath), "%s\\%s", selectedPath, items[i].name);
            AssetType type = items[i].isDir? ASSET_FOLDER : morphClassifyAsset(fullPath);
            VkDescriptorSet iconId = editor->assetIconIds[type];

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            
            if (ImGui::ImageButton(items[i].name, (ImTextureID)iconId, ImVec2(iconSize, iconSize)))
            { }
            
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (items[i].isDir)
                {
                    char newPath[MAX_PATH_LEN];
                    snprintf(newPath, sizeof(newPath), "%s\\%s", selectedPath, items[i].name);
                    navigateTo(newPath, true);
                }
            }

            ImGui::TextWrapped("%s", items[i].name);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void morphImGuiDrawFolderOverview(MorphEditor* editor)
{
    ImGui::Separator();
    drawFolderContents(editor);
}

void morphImGuiDrawTools(void)
{
    ImGui::Separator();
}

static const char* entityTypeName(EntityType type)
{
    switch(type)
    {
        case ENTITY_PLAYER: return "Player";
        case ENTITY_BLOCK: return "Block";
        default: return "Unkrown";
    }
}

void morphImGuiDrawOutliner(MorphScene* scene, MorphEditor* editor)
{
    ImGui::Separator();

    f32 footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

    ImGui::BeginChild("OutlinerList", ImVec2(0, -footerHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

    bool sceneSelected = (editor->selectionType == SELECTION_SCENE);
    if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | (sceneSelected ? ImGuiTreeNodeFlags_Selected : 0)))
    {
        if (ImGui::IsItemClicked())
            editor->selectionType = SELECTION_SCENE;

        for(u32 i = 0; i < scene->entitiesCount; i++)
        {
            bool isSelected = (editor->selectionType == SELECTION_ENTITY && editor->selectedEntity.index == i);

            ImGui::PushID(i);
            if (ImGui::Selectable(entityTypeName(scene->entitiesType[i]), isSelected))
            {
                editor->selectedEntity = (EntityHandle){ i, scene->entitiesGeneration[i] };
                editor->selectionType = SELECTION_ENTITY;
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Text("%u entities", scene->entitiesCount);
}

void morphImGuiDrawDetails(MorphScene* scene, MorphEditor* editor)
{
    ImGui::Separator();

    if (editor->selectionType == SELECTION_ENTITY)
    {
        u32 i = editor->selectedEntity.index;

        ImGui::Text("Type: %s", entityTypeName(scene->entitiesType[i]));
        ImGui::Text("Position: %.2f, %.2f", scene->entitiesPosition[i].x, scene->entitiesPosition[i].y);
        ImGui::Text("Size: %.2f, %.2f", scene->entitiesSize[i].x, scene->entitiesSize[i].y);
        ImGui::Text("Velocity: %.2f, %.2f", scene->entitiesVelocity[i].x, scene->entitiesVelocity[i].y);
        ImGui::Text("Enabled components:");
        ImGui::Text(" Position: %s", (scene->entitiesFlags[i] & COMPONENT_POSITION) ? "enabled" : "disabled");
        ImGui::Text(" Velocity: %s", (scene->entitiesFlags[i] & COMPONENT_VELOCITY) ? "enabled" : "disabled");
        ImGui::Text(" Texture: %s", (scene->entitiesFlags[i] & COMPONENT_TEXTURE)  ? "enabled" : "disabled");
        ImGui::Text(" Size: %s", (scene->entitiesFlags[i] & COMPONENT_SIZE) ? "enabled" : "disabled");
    }
    else if (editor->selectionType == SELECTION_SCENE)
    {
        ImGui::ColorEdit4("Background", (float*)&scene->settings.bgColor);
    }    
}

void morphImGuiDrawViewport(VkDescriptorSet descriptorSet, u32 texWidth, u32 texHeight)
{
    ImGui::Separator();

    ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)descriptorSet, size);
}

void morphImGuiDrawMenuBar(MorphEditor *editor, f32 deltaTime)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("Open HUB", NULL, &editor->showHUB);
            ImGui::MenuItem("Import", NULL, nullptr);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::MenuItem("Project settings", NULL, nullptr);
            ImGui::EndMenu();
        }
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
        if (ImGui::BeginMenu("Build"))
        {
            ImGui::MenuItem("Build preferences", NULL, nullptr);
            ImGui::MenuItem("Build", NULL, nullptr);
            ImGui::EndMenu();
        }

        char fpsText[32];
        snprintf(fpsText, sizeof(fpsText), "%.1f FPS  (%.2f ms)", 1.0f / deltaTime, deltaTime * 1000.0f);
        f32 textWidth = ImGui::CalcTextSize(fpsText).x;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textWidth - 10.0f);
        ImGui::Text("%s", fpsText);

        ImGui::EndMainMenuBar();
    }
}

typedef enum
{
    HUB_MAIN,
    HUB_CREATE,
    HUB_OPEN,
} hubMode;

static hubMode hub = HUB_MAIN;

void morphImGuiDrawHub(MorphEditor* editor, MorphCamera* camera, MorphScene* scene)
{
    ImGui::OpenPopup("##hub");
    ImGui::SetNextWindowSizeConstraints(ImVec2(550.0f, 0.0f), ImVec2(550.0f, FLT_MAX));
    if (ImGui::BeginPopupModal("##hub", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Morph engine");
        ImGui::TextDisabled("Recent projects");
        ImGui::Separator();

        int panelWidth = ImGui::GetContentRegionAvail().x;
        int cols = 5;
        if (cols < 1) cols = 1;

        if (hub == HUB_MAIN)
        {
            if (ImGui::BeginTable("RecentGrid", cols))
            {
                for (int i = 0; i < MAX_RECENT; i++)
                {
                    ImGui::TableNextColumn();
                    ImGui::PushID(i);

                    if (i < editor->recentCount)
                    {
                        const char* name = strrchr(editor->recentProjects[i], '\\');
                        name = name ? name + 1 : editor->recentProjects[i];

                        if (ImGui::Button(name, ImVec2(100, 100)))
                        {
                            morphEditorOpenProject(editor, camera, scene, editor->recentProjects[i]);
                        }
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
            if (ImGui::Button("New Project")) { hub = HUB_CREATE; }
            ImGui::SameLine();
            if (ImGui::Button("Open Project")) { hub = HUB_OPEN; }
        }
        else if (hub == HUB_CREATE)
        {
            static char projectName[128] = "MyGame";
            static char projectLocation[MAX_PATH_LEN] = "C:\\Users\\";

            ImGui::InputText("Name", projectName, sizeof(projectName));
            ImGui::InputText("Location", projectLocation, sizeof(projectLocation));

            if (ImGui::Button("Create"))
            {
                morphProjectCreate(&editor->project, projectName, projectLocation);
                editor->project.temporary = false;

                char mproj[MAX_PATH_LEN];
                snprintf(mproj, sizeof(mproj), "%s\\%s\\%s.mproj", projectLocation, projectName, projectName);

                morphEditorOpenProject(editor, camera, scene, mproj);
            }
            if (ImGui::Button("Cancel")) { hub = HUB_MAIN; }
        }
        else if (hub == HUB_OPEN)
        {
            char path[MAX_PATH_LEN] = {0};
            if (morphPlatformOpenFileDialog(path, MAX_PATH_LEN, "Morph project\0*.mproj\0"))
                morphEditorOpenProject(editor, camera, scene, path);
            hub = HUB_MAIN;
        }
        ImGui::EndPopup();
    }
}

Vec2 morphImGuiGetViewportSize(void)
{
    ImVec2 size = ImGui::GetContentRegionAvail();
    Vec2 result = { (f32)size.x, (f32)size.y };

    return result;
}

bool morphImGuiGetViewportFocusedCursor(void)
{ return ImGui::IsWindowHovered(); }

}