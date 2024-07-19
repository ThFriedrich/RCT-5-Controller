#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif
#include "NamurCommands.h"
#include "SerialPort.h" // Include the appropriate header file for SerialPort
#define MINI_CASE_SENSITIVE
#include "ini.h"
#include "TimeLine.h"

class RCT_5_Control
{
public:
    RCT_5_Control();
    // ~RCT_5_Control();
    void render_window(SDL_Window *window,ImGuiIO &io, SDL_GLContext &gl_context);
    
private:
    SerialPort *serialPort;
    std::vector<std::string> availablePorts;
    std::vector<uint32_t> baudRates;
    int selectedBaudRateIndex;
    int selectedPortIndex;
    bool connected;
    bool rct_detected;
    bool auto_connect;
    std::string device;

    NamurCommands namur;

    void checkAvailablePorts();
    void connectPort();
    void get_device_name();
    void send_signal(const std::string &command);
    float get_numeric_value();
    void show_command_ui();
    void show_connection_ui(mINI::INIStructure &config);
    void show_timeline_ui(TimeLine &timeline, ImGuiIO &io);
    void show_section_ui(Section &section, ImGuiIO &io);
    std::vector<TimeLine> timelines;
    
};