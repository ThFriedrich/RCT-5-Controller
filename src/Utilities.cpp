#include <sstream>
#include <iomanip>

std::string ftos(float f, int nd)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(nd) << f;
    return stream.str();
}

int bitmask2index(int bitmask) {
    int index = 0;
    while (bitmask >>= 1) {
        index++;
    }
    return index;
}