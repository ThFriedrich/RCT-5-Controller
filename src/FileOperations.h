#include "TimeLine.h"

class FileOperations
{
private:
    static void saveSection(const Section &section, std::ofstream &outFile);
    static void loadSection(Section &section, std::ifstream &inFile);

public:
    static void saveTimeLine(const TimeLine &timeline, const std::string &filename);
    static void loadTimeLine(TimeLine &timeline, const std::string &filename);
};
