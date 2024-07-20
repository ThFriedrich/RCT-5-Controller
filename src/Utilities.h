#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <string>
#include <vector>

std::string ftos(float f, int nd); // Convert float to string
int bitmask2index(int bitmask); // Convert bitmask to index
void sleep(int duration); // Sleep for duration milliseconds
#endif // UTILITIES_H