
#include "FileOperations.h"

// Serialization for Section class
void FileOperations::saveSection(const Section& section, std::ofstream& outFile) {
    size_t nameLength = section.name.size();
    outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    outFile.write(section.name.c_str(), nameLength);
    
    size_t descriptionLength = section.description.size();
    outFile.write(reinterpret_cast<const char*>(&descriptionLength), sizeof(descriptionLength));
    outFile.write(section.description.c_str(), descriptionLength);
    
    outFile.write(reinterpret_cast<const char*>(&section.duration), sizeof(section.duration));
    outFile.write(reinterpret_cast<const char*>(section.temperature), sizeof(section.temperature));
    outFile.write(reinterpret_cast<const char*>(section.speed), sizeof(section.speed));
    
    outFile.write(reinterpret_cast<const char*>(&section.wait), sizeof(section.wait));
    outFile.write(reinterpret_cast<const char*>(&section.b_beep), sizeof(section.b_beep));
    
    size_t preSectionCommandsSize = section.preSectionCommands.size();
    outFile.write(reinterpret_cast<const char*>(&preSectionCommandsSize), sizeof(preSectionCommandsSize));
    for (const auto& command : section.preSectionCommands) {
        size_t commandLength = command.size();
        outFile.write(reinterpret_cast<const char*>(&commandLength), sizeof(commandLength));
        outFile.write(command.c_str(), commandLength);
    }

    size_t postSectionCommandsSize = section.postSectionCommands.size();
    outFile.write(reinterpret_cast<const char*>(&postSectionCommandsSize), sizeof(postSectionCommandsSize));
    for (const auto& command : section.postSectionCommands) {
        size_t commandLength = command.size();
        outFile.write(reinterpret_cast<const char*>(&commandLength), sizeof(commandLength));
        outFile.write(command.c_str(), commandLength);
    }
}

// Serialization for TimeLine class
void FileOperations::saveTimeLine(const TimeLine &timeline, const std::string &filename) {
    std::ofstream outFile(filename, std::ios::binary);
    
    size_t nameLength = timeline.name.size();
    outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    outFile.write(timeline.name.c_str(), nameLength);
    
    size_t descriptionLength = timeline.description.size();
    outFile.write(reinterpret_cast<const char*>(&descriptionLength), sizeof(descriptionLength));
    outFile.write(timeline.description.c_str(), descriptionLength);
    
    outFile.write(reinterpret_cast<const char*>(&timeline.logInterval), sizeof(timeline.logInterval));
    outFile.write(reinterpret_cast<const char*>(&timeline.logTemperaturePlate), sizeof(timeline.logTemperaturePlate));
    outFile.write(reinterpret_cast<const char*>(&timeline.logSpeed), sizeof(timeline.logSpeed));
    outFile.write(reinterpret_cast<const char*>(&timeline.logViscosity), sizeof(timeline.logViscosity));
    outFile.write(reinterpret_cast<const char*>(&timeline.logTemperatureSensor), sizeof(timeline.logTemperatureSensor));
    
    
    size_t logFilePathLength = timeline.logFilePath.size();
    outFile.write(reinterpret_cast<const char*>(&logFilePathLength), sizeof(logFilePathLength));
    outFile.write(timeline.logFilePath.c_str(), logFilePathLength);
    
    size_t sectionsSize = timeline.sections.size();
    outFile.write(reinterpret_cast<const char*>(&sectionsSize), sizeof(sectionsSize));
    for (const auto& section : timeline.sections) {
        saveSection(section, outFile);
    }
}

// Deserialization for Section class
void FileOperations::loadSection(Section& section, std::ifstream& inFile) {
    size_t nameLength;
    inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
    section.name.resize(nameLength);
    inFile.read(&section.name[0], nameLength);
    
    size_t descriptionLength;
    inFile.read(reinterpret_cast<char*>(&descriptionLength), sizeof(descriptionLength));
    section.description.resize(descriptionLength);
    inFile.read(&section.description[0], descriptionLength);
    
    inFile.read(reinterpret_cast<char*>(&section.duration), sizeof(section.duration));
    inFile.read(reinterpret_cast<char*>(section.temperature), sizeof(section.temperature));
    inFile.read(reinterpret_cast<char*>(section.speed), sizeof(section.speed));
    
    inFile.read(reinterpret_cast<char*>(&section.wait), sizeof(section.wait));
    inFile.read(reinterpret_cast<char*>(&section.b_beep), sizeof(section.b_beep));
    
    size_t preSectionCommandsSize;
    inFile.read(reinterpret_cast<char*>(&preSectionCommandsSize), sizeof(preSectionCommandsSize));
    section.preSectionCommands.resize(preSectionCommandsSize);
    for (auto& command : section.preSectionCommands) {
        size_t commandLength;
        inFile.read(reinterpret_cast<char*>(&commandLength), sizeof(commandLength));
        command.resize(commandLength);
        inFile.read(&command[0], commandLength);
    }

    size_t postSectionCommandsSize;
    inFile.read(reinterpret_cast<char*>(&postSectionCommandsSize), sizeof(postSectionCommandsSize));
    section.postSectionCommands.resize(postSectionCommandsSize);
    for (auto& command : section.postSectionCommands) {
        size_t commandLength;
        inFile.read(reinterpret_cast<char*>(&commandLength), sizeof(commandLength));
        command.resize(commandLength);
        inFile.read(&command[0], commandLength);
    }
}

// Deserialization for TimeLine class
void FileOperations::loadTimeLine(TimeLine& timeline, const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    
    size_t nameLength;
    inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
    timeline.name.resize(nameLength);
    inFile.read(&timeline.name[0], nameLength);
    
    size_t descriptionLength;
    inFile.read(reinterpret_cast<char*>(&descriptionLength), sizeof(descriptionLength));
    timeline.description.resize(descriptionLength);
    inFile.read(&timeline.description[0], descriptionLength);
    
    inFile.read(reinterpret_cast<char*>(&timeline.logInterval), sizeof(timeline.logInterval));
    inFile.read(reinterpret_cast<char*>(&timeline.logTemperaturePlate), sizeof(timeline.logTemperaturePlate));
    inFile.read(reinterpret_cast<char*>(&timeline.logSpeed), sizeof(timeline.logSpeed));
    inFile.read(reinterpret_cast<char*>(&timeline.logViscosity), sizeof(timeline.logViscosity));
    inFile.read(reinterpret_cast<char*>(&timeline.logTemperatureSensor), sizeof(timeline.logTemperatureSensor));
    
    
    size_t logFilePathLength;
    inFile.read(reinterpret_cast<char*>(&logFilePathLength), sizeof(logFilePathLength));
    timeline.logFilePath.resize(logFilePathLength);
    inFile.read(&timeline.logFilePath[0], logFilePathLength);
      
    size_t sectionsSize;
    inFile.read(reinterpret_cast<char*>(&sectionsSize), sizeof(sectionsSize));
    timeline.sections.resize(sectionsSize);
    for (auto& section : timeline.sections) {
        section.timeline = &timeline;
        loadSection(section, inFile);
    }
    timeline.current_section = 0;
    timeline.running = false;
    timeline.waiting = false;
    timeline.b_stop = false;
    timeline.logData = LogData();
}