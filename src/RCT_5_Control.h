#ifndef RCT_5_CONTROL_H
#define RCT_5_CONTROL_H

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <chrono>
#include <stdio.h>
#include <SDL.h>

#include "NamurCommands.h"
#include "SerialPort.h" // Include the appropriate header file for SerialPort
#define MINI_CASE_SENSITIVE
#include "ini.h"
#include "TimeLine.h"

// Forward declarations
class Section;
class TimeLine;

class RCT_5_Control
{
public:
    RCT_5_Control();
    // ~RCT_5_Control();
    int render_window(SDL_Window *window,ImGuiIO &io, SDL_Renderer* renderer);
    void send_signal(const std::string &command);
    std::string get_response();
    float get_numeric_value();
    
private:
    SerialPort *serialPort;
    std::vector<std::string> availablePorts;
    std::vector<uint32_t> baudRates;
    size_t selectedBaudRateIndex;
    size_t selectedPortIndex;
    bool connected;
    bool rct_detected;
    bool auto_connect;
    std::string device;

    NamurCommands namur;

    void checkAvailablePorts();
    void connectPort();
    void get_device_name();
    void show_command_ui();
    void show_connection_ui(mINI::INIStructure &config);
    void show_timeline_ui(TimeLine &timeline, ImGuiIO &io);
    void show_section_ui(Section &section, ImGuiIO &io);
    std::vector<TimeLine> timelines;
    void inline save_timeline_ui(TimeLine &timeline);
    
};
#endif // RCT_5_CONTROL_H