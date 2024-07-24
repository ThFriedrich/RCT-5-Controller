#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

std::string ftos(float f, int nd); // Convert float to string
int bitmask2index(int bitmask); // Convert bitmask to index
void sleep(int duration); // Sleep for duration milliseconds
void draw_circle(const char color, const char* label); // Draw a circle with the specified color
float v_min(const std::vector<float> &v); // Find the minimum value in a vector
float v_max(const std::vector<float> &v); // Find the maximum value in a vector


#endif // UTILITIES_H