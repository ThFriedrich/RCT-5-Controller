#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "imgui.h"

std::string ftos(float f, int nd); // Convert float to string
int bitmask2index(int bitmask); // Convert bitmask to index
void sleep(int duration); // Sleep for duration milliseconds
void draw_circle(const char color, const char* label); // Draw a circle with the specified color
float v_min(const std::vector<float> &v); // Find the minimum value in a vector
float v_max(const std::vector<float> &v); // Find the maximum value in a vector
bool imgui_autosizingMultilineInput(const char* label, std::string* str, const ImVec2 &sizeMin, const ImVec2 &sizeMax, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None); // Autosizing multiline input
#endif // UTILITIES_H