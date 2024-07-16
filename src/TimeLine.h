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

    std::vector<std::string> preSectionCommands; // Commands to execute before the section
    std::vector<std::string> postSectionCommands; // Commands to execute after the section

    Section(size_t duration, size_t temperature[2], size_t speed[2], SectionType type)
        : duration(duration), temperature(), speed(), type(type) {}
};

// TimeLine class definition
class TimeLine {
private:
    std::vector<Section> sections;
    int logInterval; // Interval for logging in seconds
    std::vector<std::string> logCommands; // Commands to execute for logging

public:
    TimeLine(int logInterval) : logInterval(logInterval) {}

    void addSection(const Section& section) {
        sections.push_back(section);
    }

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