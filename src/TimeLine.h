#ifndef TIMELINE_H
#define TIMELINE_H

#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include "NamurCommands.h" // Include the NamurCommands header

enum class SectionType {
    Ramp,
    Static
};

// Section class definition
class Section {
public:
    size_t duration; // Duration in seconds
    size_t temperature[2]; // Temperature in degrees Celsius (beginning and end of the section)
    size_t speed[2]; // Speed in RPM (beginning and end of the section)
    SectionType type; // Type of the section (Ramp or Static)
    std::string name; // Name of the section
    std::string description; // Description of the section
    bool wait; // Wait for user input before proceeding to the next section
    
    std::vector<std::string> preSectionCommands; // Commands to execute before the section
    std::vector<std::string> postSectionCommands; // Commands to execute after the section

    Section(size_t duration, size_t temperature[2], size_t speed[2], SectionType type)
        : duration(duration), temperature(), speed(), type(type) {}
    
    Section() : duration(10), temperature(), speed(), type(SectionType::Static) {}
    Section(std::string name) : duration(10), temperature(), speed(), type(SectionType::Static), name(name) {}
};

// TimeLine class definition
class TimeLine {
public:
    std::string name;
    std::string description; // Description of the timeline
    std::vector<Section> sections;
    size_t logInterval; // Interval for logging in seconds
    bool logTemperaturePlate; // Log temperature plate readings
    bool logSpeed; // Log speed readings
    bool logViscosity; // Log viscosity readings
    bool logTemperatureSensor; // Log temperature sensor readings

    std::vector<std::string> logCommands; // Commands to execute for logging
    TimeLine(int logInterval) : logInterval(logInterval) {}
    TimeLine(std::string name) : logInterval(10), name(name) {}

    void addSection(const Section& section);

    // void execute() {
    //     NamurCommands& commands = NamurCommands::getInstance(); // Get the instance of NamurCommands
    //     for (const Section& section : sections) {
    //         // Set temperature and speed for the section
    //         commands.to_string("OUT_SP_1 " + std::to_string(section.temperature)); // Example command
    //         commands.to_string("OUT_SP_4 " + std::to_string(section.speed)); // Example command

    //         // Wait for the duration of the section
    //         std::this_thread::sleep_for(std::chrono::seconds(section.duration));

    //         // Log readings at the specified interval
    //         for (int elapsed = 0; elapsed < section.duration; elapsed += logInterval) {
    //             // Read back values from the device and log
    //             std::cout << "Logging values..." << std::endl; // Placeholder for actual logging
    //             std::this_thread::sleep_for(std::chrono::seconds(logInterval));
    //         }
    //     }
    // }
};

#endif // TIMELINE_H