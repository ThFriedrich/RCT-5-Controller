#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include "imgui.h"
#include "imgui_stdlib.h"
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

// Add soft returns to text for multiline text wrapping
// from https://github.com/ocornut/imgui/issues/3237

bool imgui_autosizingMultilineInput(const char* label, std::string* str, const ImVec2& sizeMin, const ImVec2& sizeMax, ImGuiInputTextFlags flags) {

    // calculate the maximum y/height
	ImGui::PushTextWrapPos(sizeMax.x);
	auto textSize = ImGui::CalcTextSize(str->c_str());
	if (textSize.x > sizeMax.x) {
		float ratio = textSize.x / sizeMax.x;
		textSize.x = sizeMax.x;
		textSize.y *= ratio;
		textSize.y += 20;		// add space for an extra line
	}

	textSize.y += 8;		// to compensate for inputbox margins

	if (textSize.x < sizeMin.x)
		textSize.x = sizeMin.x;
	if (textSize.y < sizeMin.y)
		textSize.y = sizeMin.y;
	if (textSize.x > sizeMax.x)
		textSize.x = sizeMax.x;
	if (textSize.y > sizeMax.y)
		textSize.y = sizeMax.y;

	bool value_changed = ImGui::InputTextMultiline(label, str, textSize, flags);

	ImGui::PopTextWrapPos();

	return value_changed;
}