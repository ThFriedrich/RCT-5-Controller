#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <imgui.h>
#include "Utilities.h"

std::string ftos(float f, int nd)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(nd) << f;
    return stream.str();
}

int bitmask2index(int bitmask)
{
    int index = 0;
    while (bitmask >>= 1)
    {
        index++;
    }
    return index;
}

void sleep(int duration)
{
    std::this_thread::sleep_for(
        std::chrono::milliseconds(duration));
}

void draw_circle(const char color, const char *label)
{
    auto ImGuiCol = IM_COL32(255, 0, 0, 255);
    if (color == 'r')
    {
        ImGuiCol = IM_COL32(255, 0, 0, 255);
    }
    else if (color == 'g')
    {
        ImGuiCol = IM_COL32(0, 255, 0, 255);
    }
    else if (color == 'b')
    {
        ImGuiCol = IM_COL32(0, 0, 255, 255);
    }
    else if (color == 'y')
    {
        ImGuiCol = IM_COL32(255, 255, 0, 255);
    }
    ImGui::Spacing();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    const float h = ImGui::GetTextLineHeight();
    const ImVec2 p1 = ImGui::GetCursorScreenPos();
    draw_list->AddCircleFilled(ImVec2(p1.x + h / 2, p1.y + h / 2), 5.0f, ImGuiCol);
    ImGui::Dummy(ImVec2(h, h));
    ImGui::SameLine();
    ImGui::Text(label);
}

float v_min(const std::vector<float> &v)
{
    // Find the minimum value in a vector
    if (v.empty())
    {
        return 0;
    }
    else
    {
        return *std::min_element(v.begin(), v.end());
    }
}

float v_max(const std::vector<float> &v)
{
        // Find the maximum value in a vector
    if (v.empty())
    {
        return 1;
    }
    else
    {
        return *std::max_element(v.begin(), v.end());
    }
}