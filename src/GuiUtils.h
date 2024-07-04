#include "imgui.h"
#include "imgui_internal.h"

class Main_Dock
{
public:
    ImGuiID dock_id;
    const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace;
    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    explicit Main_Dock(ImGuiID dock_id);
    void render(ImVec2 pos, ImVec2 size);
};
