#ifndef TIMELINE_H
#define TIMELINE_H

#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include "NamurCommands.h" // Include the NamurCommands header
#include "SerialPort.h"    // Include the appropriate header file for SerialPort
#include <ios>
#include <fstream>
#include "RCT_5_Control.h"
#include "Utilities.h"

// Forward declarations
class Section;
class RCT_5_Control;

// Internal LogData class definition
class LogData
{
public:
    std::vector<float> time;
    std::vector<float> temperaturePlate;
    std::vector<float> temperatureSensor;
    std::vector<float> speed;
    std::vector<float> viscosity;

    LogData() : time(), temperaturePlate(), temperatureSensor(), speed(), viscosity() {}
    void addData(float t, float tp, float ts, float s, float v);
};

// TimeLine class definition
class TimeLine
{
private:
    void execute_thread();

public:
    std::string name;                     // Name of the timeline
    std::string description;              // Description of the timeline
    std::vector<Section> sections;        // Sections of the timeline
    size_t logInterval;                   // Interval for logging in seconds
    bool logTemperaturePlate;             // Log temperature plate readings
    bool logSpeed;                        // Log speed readings
    bool logViscosity;                    // Log viscosity readings
    bool logTemperatureSensor;            // Log temperature sensor readings
    std::vector<std::string> logCommands; // Commands to execute for logging
    std::thread *communication_thread;    // Thread for communication with the device
    std::string logFilePath;              // Path of the log file
    RCT_5_Control *rct;                   // Pointer to the RCT_5_Control object
    bool b_stop;                          // Stop the timeline manually
    bool waiting;                         // Waiting for user input
    bool running;                         // Run status the timeline
    size_t current_section;               // Current section index
    LogData logData;                      // Log data for the timeline

    TimeLine(std::string name, RCT_5_Control *rct) : name(name), description(), sections(),
                                                     logInterval(10), logTemperaturePlate(true), logSpeed(true),
                                                     logViscosity(true), logTemperatureSensor(true),
                                                     communication_thread(nullptr), logFilePath(),
                                                     rct(rct), b_stop(false), waiting(false), running(false),
                                                     current_section(0), logData() {}

    ~TimeLine();
    void addSection(const Section &section);
    void execute();
    void stop();
};

// Section class definition
class Section
{
private:
    std::vector<float> temperatures;
    std::vector<float> speeds;
    size_t interval;
    SerialPort *serialPort;
    TimeLine *timeline;
    void compile_section();

public:
    size_t duration;                              // Duration in seconds
    uint16_t temperature[2];                      // Temperature in degrees Celsius (beginning and end of the section)
    uint16_t speed[2];                            // Speed in RPM (beginning and end of the section)
    std::string name;                             // Name of the section
    std::string description;                      // Description of the section
    bool wait;                                    // Wait for user input before proceeding to the next section
    bool b_beep;                                  // Sound a beep at end beginning of the section
    std::vector<std::string> preSectionCommands;  // Commands to execute before the section
    std::vector<std::string> postSectionCommands; // Commands to execute after the section

    Section(std::string name, TimeLine *timeline) : temperatures(), speeds(), duration(5), temperature{30, 30}, speed{0, 0}, name(name), description(), wait(false), b_beep(false), timeline(timeline) {}

    void execute_section();
    void sound_beep();
};

#endif // TIMELINE_H