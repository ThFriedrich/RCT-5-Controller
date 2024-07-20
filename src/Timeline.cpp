#include "TimeLine.h"
#include "beeper.h"

void TimeLine::addSection(const Section &section)
{
    sections.push_back(section);
}

void Section::sound_beep()
{
    std::thread t1 = std::thread(Beeper::super_mario_level_finshed, 0.75);
    t1.detach();
}