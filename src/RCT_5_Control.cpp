#include "RCT_5_Control.h"
#include "SerialPort.h"
#include <vector>
#include <string>
#include <bit>
#include "imgui.h"
#include "imgui_internal.h"

#include "ImGuiINI.hpp"
#define MINI_CASE_SENSITIVE
#include "ini.h"
#include "Utilities.h"
#include "imgui_stdlib.h"

RCT_5_Control::RCT_5_Control()
{
    serialPort = nullptr;
    availablePorts = listSerialPorts();
    device = "No device detected";
    connected = false;
    rct_detected = false;
    selectedPortIndex = -1;
    baudRates = {4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    selectedBaudRateIndex = 2;
    auto_connect = false;
}

static std::string statusMessage = "No serial port connected";
static int selectedCommandIndex = -1;
static int font_index = 0;
static int style_index = 0;

static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable & ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNav;
static ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

void RCT_5_Control::checkAvailablePorts()
{
    availablePorts = listSerialPorts();
}
void RCT_5_Control::connectPort()
{
    if (selectedPortIndex < 0)
    {
        statusMessage = "No port selected";
        connected = false;
        return;
    }
    if (serialPort)
    {
        delete serialPort;
    }
    serialPort = new SerialPort(availablePorts[selectedPortIndex], baudRates[selectedBaudRateIndex]);
    if (serialPort->open())
    {
        statusMessage = "Connected to serial port";
        connected = true;
    }
    else
    {
        statusMessage = "Failed to connect to serial port";
        connected = false;
    }
}
void RCT_5_Control::get_device_name()
{
    device = "No device detected";
    rct_detected = false;
    if (serialPort && connected)
    {
        if (serialPort->sendCommand("IN_NAME"))
        {
            std::string temp_device = serialPort->readString();
            if (temp_device.size() > 0)
            {
                device = temp_device;
                rct_detected = true;
            }
        }
    }
}
float RCT_5_Control::get_numeric_value()
{
    if (namur.responseText.size() > 0)
    {
        std::string value = namur.responseText;
        value.erase(std::find_if(value.begin(), value.end(), [](char c)
                                 { return std::isspace(c); }),
                    value.end());
        return std::stof(value);
    }
    else
    {
        return 0.0f;
    }
}
void RCT_5_Control::send_signal(const std::string &command)
{
    if (serialPort && connected)
    {
        if (serialPort->sendCommand(command))
        {
            namur.responseText = serialPort->readString();
            if (namur.responseText.size() <= 0)
            {
                namur.responseText = "Failed to read from serial port";
            }
        }
        else
        {
            namur.responseText = "Failed to send signal";
        }
    }
    else
    {
        namur.responseText = "Serial port not connected";
    }
}
void RCT_5_Control::show_connection_ui(mINI::INIStructure &config)
{
    if (ImGui::Button("Refresh Ports", ImVec2(-1, 0)))
    {
        checkAvailablePorts();
    }
    if (ImGui::BeginCombo("Serial Ports", selectedPortIndex >= 0 ? availablePorts[selectedPortIndex].c_str() : "Select a port"))
    {
        for (size_t i = 0; i < availablePorts.size(); ++i)
        {
            bool isSelected = (selectedPortIndex == i);
            if (ImGui::Selectable(availablePorts[i].c_str(), isSelected))
            {
                selectedPortIndex = i;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
                config["Settings"]["COM-Port"] = std::to_string(selectedPortIndex);
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Baud Rate", std::to_string(baudRates[selectedBaudRateIndex]).c_str()))
    {
        for (size_t i = 0; i < baudRates.size(); ++i)
        {
            bool isSelected = (selectedBaudRateIndex == i);
            if (ImGui::Selectable(std::to_string(baudRates[i]).c_str(), isSelected))
            {
                selectedBaudRateIndex = i;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
                config["Settings"]["Baud-Rate"] = std::to_string(selectedBaudRateIndex);
            }
        }
        ImGui::EndCombo();
    }
    // Connect button
    if (ImGui::Button("Connect"))
    {
        connectPort();
        get_device_name();
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Reconnect on startup", &auto_connect))
    {
        config["Settings"]["Reconnect"] = std::to_string(auto_connect);
    }
    ImGui::TextColored(connected ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), "Status: %s", statusMessage.c_str());
    if (rct_detected)
    {
        ImGui::Text("Device: %s", device.c_str());
    }
}
void RCT_5_Control::show_timeline_ui(TimeLine &timeline)
{
    ImGui::Text("Timeline: %s", timeline.name.c_str());
    if (ImGui::Button("New Section"))
    {
        timeline.sections.push_back(Section("Section " + std::to_string(timeline.sections.size() + 1)));
    }
}
void RCT_5_Control::show_section_ui(Section &section)
{
    ImGui::Text(section.name.c_str());
    static bool demo = false;
    ImGui::Checkbox("Demo", &demo);
    if (demo)
    {
        ImGui::ShowDemoWindow();
    }

}
void RCT_5_Control::render_window(SDL_Window *window, ImGuiIO &io, SDL_GLContext &gl_context)
{
    // Main loop
    bool done = false;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    // INI file settings to restore previous session settings
    mINI::INIFile ini_file("imgui.ini");
    mINI::INIStructure ini_cfg;
    ini_file.read(ini_cfg);

    // Appearance
    static float fontscale = 1.0f;
    ImGuiINI::check_ini_setting(ini_cfg, "Appearance", "Font", font_index);
    ImGuiINI::set_font(font_index);
    ImGuiINI::check_ini_setting(ini_cfg, "Appearance", "Font Scale", fontscale);
    io.FontGlobalScale = fontscale;
    ImGuiINI::check_ini_setting(ini_cfg, "Appearance", "Style", style_index);
    ImGuiINI::set_style(style_index);
    ImGuiINI::check_ini_setting(ini_cfg, "Settings", "COM-Port", selectedPortIndex);
    ImGuiINI::check_ini_setting(ini_cfg, "Settings", "Baud-Rate", selectedBaudRateIndex);
    ImGuiINI::check_ini_setting(ini_cfg, "Settings", "Reconnect", auto_connect);

    if (auto_connect)
    {
        connectPort();
        get_device_name();
    }

    while (!done)

    {
        // Poll and handle events (inputs, window resize, etc.)
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Main Menu Bar
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Appearance"))
            {
                ImGuiINI::ShowFontSelector("Font", font_index, ini_cfg);
                ImGuiINI::ShowStyleSelector("Style", style_index, ini_cfg);
                if (ImGui::DragFloat("Scale Font", &io.FontGlobalScale, 0.005f, 0.3f, 1.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
                {
                    ini_cfg["Appearance"]["Font Scale"] = std::to_string(io.FontGlobalScale);
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Connection"))
            {
                show_connection_ui(ini_cfg);
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();

        // Make the window use the entire work area
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        // Main Window with Tab environment
        ImGui::Begin("RCT 5 Interface", NULL, window_flags);

        if (ImGui::BeginTabBar("TabBar1", tab_bar_flags))
        {

            if (ImGui::BeginTabItem("Direct Interface", NULL, ImGuiTabItemFlags_None))
            {
                show_connection_ui(ini_cfg);
                if (ImGui::BeginCombo("Commands", selectedCommandIndex >= 0 ? namur.getCommandDetails(namur[selectedCommandIndex]).description.c_str() : "Select a command"))
                {
                    for (int i = 0; i < namur.size(); ++i)
                    {
                        bool isSelected = (selectedCommandIndex == i);
                        std::string command = namur.getCommandDetails(namur[i]).description;
                        if (ImGui::Selectable(command.c_str(), isSelected))
                        {
                            selectedCommandIndex = i;
                        }
                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip(namur[i].c_str());
                        }
                    }
                    ImGui::EndCombo();
                }
                if (selectedCommandIndex >= 0)
                {
                    auto commandDetails = namur.getCommandDetails(namur[selectedCommandIndex]);
                    if (commandDetails.requiresValue)
                    {
                        ImGui::InputScalar("Parameter", ImGuiDataType_U16, &namur.parameter);
                    }

                    // Send signal button
                    if (ImGui::Button("Send Signal"))
                    {
                        send_signal(namur.to_string(namur[selectedCommandIndex]));
                        if (namur[selectedCommandIndex] != "IN_NAME" && selectedCommandIndex >= 0 && namur.getCommandDetails(namur[selectedCommandIndex]).returnsValue)
                        {
                            float responseFloat = get_numeric_value();
                            namur.responseText = ftos(responseFloat, 1);
                        }
                    }
                }
                // Response text box
                ImGui::InputText("Response", &namur.responseText, ImGuiInputTextFlags_ReadOnly);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Script Editor", NULL, ImGuiTabItemFlags_None))
            {
                // Tree view for timelines and sections
                static int timeline_mask = (1 << 0);
                static int section_mask = (1 << 0);
                int timeline_clicked = -1;
                int section_clicked = -1;
                static bool last_click_section = true; // true if last click was on a section
                ImGui::BeginColumns("columns", 2);
                {
                    static bool first_time = true;
                    if (first_time)
                    {
                        ImGui::SetColumnWidth(0, ImGui::GetContentRegionAvail().x * 0.2);
                        first_time = false;
                    }
                    ImGui::BeginChild("Script Editor", ImVec2(-1, -1), ImGuiChildFlags_None);
                    if (ImGui::Button("New Timeline"))
                    {
                        timelines.push_back(TimeLine("Timeline " + std::to_string(timelines.size() + 1)));
                        timeline_mask = (1 << (timelines.size() - 1));
                    }
                    if (timelines.size() > 0)
                    {
                        // 'selection_mask' is dumb representation of what may be user-side selection state.
                        //  You may retain selection state inside or outside your objects in whatever format you see fit.
                        // 'node_clicked' is temporary storage of what node we have clicked to process selection at the end
                        /// of the loop. May be a pointer to your own node type, etc.

                        for (size_t i = 0; i < timelines.size(); i++)
                        {
                            // Disable the default "open on single-click behavior" + set Selected flag according to our selection.
                            // To alter selection we use IsItemClicked() && !IsItemToggledOpen(), so clicking on an arrow doesn't alter selection.
                            ImGuiTreeNodeFlags tl_node_flags = tree_flags;
                            const bool tl_is_selected = (timeline_mask & (1 << i)) != 0;
                            if (tl_is_selected)
                                tl_node_flags |= ImGuiTreeNodeFlags_Selected;

                            bool node_open = ImGui::TreeNodeEx(timelines[i].name.c_str(), tl_node_flags, timelines[i].name.c_str());
                            if (ImGui::IsItemClicked())
                            {
                                timeline_clicked = i;
                                last_click_section = false;
                            }

                            if (node_open)
                            {
                                for (size_t j = 0; j < timelines[i].sections.size(); j++)
                                {
                                    ImGuiTreeNodeFlags sec_node_flags = tree_flags;
                                    const bool se_is_selected = (section_mask & (1 << j)) != 0;
                                    if (se_is_selected && tl_is_selected && last_click_section)
                                        sec_node_flags |= ImGuiTreeNodeFlags_Selected;
                                    sec_node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                                    ImGui::TreeNodeEx(timelines[i].sections[j].name.c_str(), sec_node_flags, timelines[i].sections[j].name.c_str());
                                    if (ImGui::IsItemClicked())
                                    {
                                        section_clicked = j;
                                        timeline_clicked = i;
                                        last_click_section = true;
                                    }
                                }
                                if (section_clicked != -1)
                                {
                                    // Update selection state
                                    section_mask = (1 << section_clicked); // Click to single-select
                                }
                                ImGui::TreePop();
                            }
                        }
                        if (timeline_clicked != -1)
                        {
                            // Update selection state
                            timeline_mask = (1 << timeline_clicked); // Click to single-select
                        }
                    }
                    ImGui::EndChild();
                    ImGui::NextColumn();
                    int index_tl = bitmask2index(timeline_mask);
                    int index_sec = bitmask2index(section_mask);

                    ImGui::Text("Selected Timeline Index %d", index_tl);
                    ImGui::Text("Selected Section Index %d", index_sec);
                    if (!last_click_section)
                    {
                        show_timeline_ui(timelines[index_tl]);
                    }
                    else if (timelines.size() > 0 && timelines[index_tl].sections.size() > 0)
                    {
                        show_section_ui(timelines[index_tl].sections[index_sec]);
                    }

                    ImGui::EndColumns();
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Script Runner", NULL, ImGuiTabItemFlags_None))
            {
                ImGui::Text("Hello, other world!");
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();

        ImGui::End();

        // Rendering
        ImGui::Render();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // Save INI file settings
    ini_file.write(ini_cfg);
}