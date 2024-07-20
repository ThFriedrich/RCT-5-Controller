#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

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