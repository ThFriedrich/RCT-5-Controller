#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <string>
#include <vector>

std::string ftos(float f, int nd); // Convert float to string
int bitmask2index(int bitmask); // Convert bitmask to index
void sleep(int duration); // Sleep for duration milliseconds
void draw_circle(const char color, const char* label); // Draw a circle with the specified color
#endif // UTILITIES_H