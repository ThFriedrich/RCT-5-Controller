#include "RCT_5_Control.h"
#include "SerialPort.h"
#include <vector>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include <imfilebrowser.h>

#include "ImGuiINI.hpp"
#define MINI_CASE_SENSITIVE
#include "ini.h"
#include "Utilities.h"
#include "imgui_stdlib.h"

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

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
static size_t selectedCommandIndex = -1;
static int font_index = 0;
static int style_index = 0;
static uint16_t timeline_index = -1;

static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable & ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNav;
static ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

void RCT_5_Control::checkAvailablePorts()
{
    availablePorts = listSerialPorts();
}
void RCT_5_Control::connectPort()
{
    if (selectedPortIndex > availablePorts.size())
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
    std::string base_command = namur.get_base_command(command);
    NamurCommands::CommandDetails comDetails = namur.getCommandDetails(base_command);

    if (serialPort && connected)
    {
        if (serialPort->sendCommand(command))
        {
            if (comDetails.returnsValue)
            {
                namur.responseText = serialPort->readString();
                if (namur.responseText.size() <= 0)
                {
                    namur.responseText = "Failed to read from serial port";
                }
                else if (base_command != "IN_NAME")
                {
                    float responseFloat = get_numeric_value();
                    namur.responseText = ftos(responseFloat, 1);
                }
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
    if (ImGui::BeginCombo("Serial Ports", selectedPortIndex < availablePorts.size() ? availablePorts[selectedPortIndex].c_str() : "Select a port"))
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
    ImGui::Text("Status: %s", statusMessage.c_str());
    if (rct_detected)
    {
        ImGui::Text("Device: %s", device.c_str());
    }
    if (connected && rct_detected)
    {
        draw_circle('g', "RCT 5 Connected");
    }
    else
    {
        draw_circle('r', "RCT 5  not connected");
    }
}
void RCT_5_Control::show_command_ui()
{
    if (ImGui::BeginCombo("Commands", selectedCommandIndex < namur.n ? namur.getCommandDetails(namur[selectedCommandIndex]).description.c_str() : "Select a command"))
    {
        for (size_t i = 0; i < namur.size(); ++i)
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
    if (selectedCommandIndex < namur.n)
    {
        auto commandDetails = namur.getCommandDetails(namur[selectedCommandIndex]);
        if (commandDetails.requiresValue)
        {
            ImGui::InputScalar("Parameter", ImGuiDataType_U16, &namur.parameter);
        }
    }
}
void RCT_5_Control::show_timeline_ui(TimeLine &timeline, ImGuiIO &io)
{
    ImGui::PushFont(io.Fonts->Fonts[font_index + 6]);
    ImGui::InputText("Name", &timeline.name);
    ImGui::PopFont();
    ImGui::InputTextMultiline("Description", &timeline.description);
    ImGui::InputScalar("Log Interval", ImGuiDataType_U64, &timeline.logInterval);

    ImGui::Checkbox("Log Temperature (Plate)", &timeline.logTemperaturePlate);
    ImGui::SameLine();
    ImGui::Checkbox("Log Temperature (Sensor)", &timeline.logTemperatureSensor);
    ImGui::SameLine();
    ImGui::Checkbox("Log Speed", &timeline.logSpeed);
    ImGui::SameLine();
    ImGui::Checkbox("Log Viscosity Trend", &timeline.logViscosity);

    if (ImGui::Button("New Section"))
    {
        timeline.sections.push_back(Section("Section " + std::to_string(timeline.sections.size() + 1), &timeline));
    }
    if (timeline.sections.size() > 0)
    {
        ImGui::BeginTable("Sections", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable, ImVec2(-1, -1));
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Duration");
        ImGui::TableSetupColumn("Temperature Start");
        ImGui::TableSetupColumn("Temperature End");
        ImGui::TableSetupColumn("Speed Start");
        ImGui::TableSetupColumn("Speed End");
        ImGui::TableSetupColumn("Wait");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < timeline.sections.size(); i++)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(("##Name" + std::to_string(i)).c_str(), &timeline.sections[i].name);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputScalar(("##Duration" + std::to_string(i)).c_str(), ImGuiDataType_U64, &timeline.sections[i].duration);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputScalar(("##TempStart" + std::to_string(i)).c_str(), ImGuiDataType_U16, &timeline.sections[i].temperature[0]);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputScalar(("##TempEnd" + std::to_string(i)).c_str(), ImGuiDataType_U16, &timeline.sections[i].temperature[1]);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputScalar(("##SpeedStart" + std::to_string(i)).c_str(), ImGuiDataType_U16, &timeline.sections[i].speed[0]);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputScalar(("##SpeedEnd" + std::to_string(i)).c_str(), ImGuiDataType_U16, &timeline.sections[i].speed[1]);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::Checkbox(("##Wait" + std::to_string(i)).c_str(), &timeline.sections[i].wait);
        }
        ImGui::EndTable();
    }
}
void RCT_5_Control::show_section_ui(Section &section, ImGuiIO &io)
{
    ImGui::PushFont(io.Fonts->Fonts[font_index + 6]);
    ImGui::InputText("Section", &section.name);
    ImGui::PopFont();
    ImGui::SetItemTooltip("Name of the section");
    ImGui::InputTextMultiline("Description", &section.description);
    ImGui::SetItemTooltip("Description of the section");
    ImGui::InputScalar("Duration", ImGuiDataType_U64, &section.duration);
    ImGui::SetItemTooltip("Duration in seconds");
    ImGui::InputScalar("Temperature Start", ImGuiDataType_U16, &section.temperature[0]);
    ImGui::SetItemTooltip("Temperature at the beginning of the section in degrees Celsius");
    ImGui::InputScalar("Temperature End", ImGuiDataType_U16, &section.temperature[1]);
    ImGui::SetItemTooltip("Temperature at the end of the section in degrees Celsius");
    ImGui::InputScalar("Speed Start", ImGuiDataType_U16, &section.speed[0]);
    ImGui::SetItemTooltip("Rotation speed at the beginning of the section in RPM");
    ImGui::InputScalar("Speed End", ImGuiDataType_U16, &section.speed[1]);
    ImGui::SetItemTooltip("Rotation speed at the end of the section in RPM");
    ImGui::SetItemTooltip("Wait for user input before proceeding to the next section");
    ImGui::Checkbox("Wait", &section.wait);
    ImGui::SetItemTooltip("Sound a beep at the end of the section");
    ImGui::Checkbox("Beep on completion", &section.b_beep);

    if (ImGui::Button("Add Pre-Section Command"))
    {
        if (selectedCommandIndex < namur.n)
            section.preSectionCommands.push_back(namur.to_string(namur[selectedCommandIndex]));
    }
    ImGui::SetItemTooltip("Commands to execute before the section");
    ImGui::SameLine();
    if (ImGui::Button("Add Post-Section Command"))
    {
        if (selectedCommandIndex < namur.n)
            section.postSectionCommands.push_back(namur.to_string(namur[selectedCommandIndex]));
    }
    ImGui::SetItemTooltip("Commands to execute after the section");

    show_command_ui();

    if (section.preSectionCommands.size() > 0)
    {
        ImGui::BeginTable("Pre-Section Commands", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable, ImVec2(-1, 0));
        ImGui::TableSetupColumn("Pre-Section Commands", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Modify", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < section.preSectionCommands.size(); i++)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::TextUnformatted(section.preSectionCommands[i].c_str());
            std::string base_command = namur.get_base_command(section.preSectionCommands[i]);
            ImGui::SetItemTooltip(namur.getCommandDetails(base_command).description.c_str());
            ImGui::TableNextColumn();
            if (ImGui::Button(("Move up##PreCommand" + std::to_string(i)).c_str()))
            {
                if (i > 0)
                {
                    std::swap(section.preSectionCommands[i], section.preSectionCommands[i - 1]);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(("Move down##PreCommand" + std::to_string(i)).c_str()))
            {
                if (i < section.preSectionCommands.size() - 1)
                {
                    std::swap(section.preSectionCommands[i], section.preSectionCommands[i + 1]);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(("Delete##PreCommand" + std::to_string(i)).c_str()))
            {
                section.preSectionCommands.erase(section.preSectionCommands.begin() + i);
            }
        }
        ImGui::EndTable();
    }

    if (section.postSectionCommands.size() > 0)
    {
        ImGui::BeginTable("Post-Section Commands", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable, ImVec2(-1, 0));
        ImGui::TableSetupColumn("Post-Section Commands", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Modify", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < section.postSectionCommands.size(); i++)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::Text(section.postSectionCommands[i].c_str());
            std::string base_command = namur.get_base_command(section.postSectionCommands[i]);
            ImGui::SetItemTooltip(namur.getCommandDetails(base_command).description.c_str());
            ImGui::TableNextColumn();
            if (ImGui::Button(("Move up##PostCommand" + std::to_string(i)).c_str()))
            {
                if (i > 0)
                {
                    std::swap(section.postSectionCommands[i], section.postSectionCommands[i - 1]);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(("Move down##PostCommand" + std::to_string(i)).c_str()))
            {
                if (i < section.postSectionCommands.size() - 1)
                {
                    std::swap(section.postSectionCommands[i], section.postSectionCommands[i + 1]);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(("Delete##PostCommand" + std::to_string(i)).c_str()))
            {
                section.postSectionCommands.erase(section.postSectionCommands.begin() + i);
            }
        }
        ImGui::EndTable();
    }
}
std::string RCT_5_Control::get_response()
{
    return namur.responseText;
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
    ImGui::FileBrowser fileDialog(ImGuiFileBrowserFlags_EnterNewFilename);
    fileDialog.SetTitle("Log File");
    fileDialog.SetCurrentDirectory(".");
    fileDialog.SetInputName("rct_log.txt");

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
                if (ImGui::DragFloat("Scale Font", &io.FontGlobalScale, 0.005f, 0.5f, 2.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
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
                if (ImGui::BeginCombo("Commands", selectedCommandIndex < namur.n ? namur.getCommandDetails(namur[selectedCommandIndex]).description.c_str() : "Select a command"))
                {
                    for (size_t i = 0; i < namur.size(); ++i)
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
                if (selectedCommandIndex < namur.n)
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
                        ImGui::SetColumnWidth(0, ImGui::GetContentRegionAvail().x * 0.3);
                        first_time = false;
                    }
                    ImGui::BeginChild("Script Editor", ImVec2(-1, -1), ImGuiChildFlags_None);
                    if (ImGui::Button("New Timeline"))
                    {
                        timelines.push_back(TimeLine("Timeline " + std::to_string(timelines.size() + 1), this));
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

                    // ImGui::Text("Selected Timeline Index %d", index_tl);
                    // ImGui::Text("Selected Section Index %d", index_sec);
                    if (!last_click_section)
                    {
                        show_timeline_ui(timelines[index_tl], io);
                    }
                    else if (timelines.size() > 0 && timelines[index_tl].sections.size() > 0)
                    {
                        show_section_ui(timelines[index_tl].sections[index_sec], io);
                    }

                    ImGui::EndColumns();
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Script Runner", NULL, ImGuiTabItemFlags_None))
            {

                if (connected && rct_detected)
                {
                    draw_circle('g', "RCT 5 Connected");
                }
                else
                {
                    show_connection_ui(ini_cfg);
                }

                if (ImGui::BeginCombo("Timelines", timeline_index < timelines.size() ? timelines[timeline_index].name.c_str() : "Select a timeline"))
                {
                    for (size_t i = 0; i < timelines.size(); ++i)
                    {
                        bool isSelected = (timeline_index == i);
                        if (ImGui::Selectable(timelines[i].name.c_str(), isSelected))
                        {
                            timeline_index = i;
                        }
                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                if (timeline_index < timelines.size())
                {
                    if (ImGui::Button("Specify Log file path"))
                        fileDialog.Open();
                    fileDialog.Display();
                    if (fileDialog.HasSelected())
                    {
                        timelines[timeline_index].logFilePath = fileDialog.GetSelected().string();
                        fileDialog.ClearSelected();
                    }
                    ImGui::SameLine();
                    ImGui::InputText("Log Path: ", &timelines[timeline_index].logFilePath);

                    if (ImGui::Button("Run Script"))
                    {
                        if (timeline_index < timelines.size())
                        {
                            timelines[timeline_index].execute();
                        }
                    }
                    if ( timelines[timeline_index].running)
                    {
                        ImGui::SameLine();
                        if (ImGui::Button("Stop Script"))
                        {
                            if (timeline_index < timelines.size())
                            {
                                timelines[timeline_index].stop();
                            }
                        }
                        ImGui::Text("Running script: %s", timelines[timeline_index].name.c_str());
                        size_t current_section = timelines[timeline_index].current_section;
                        ImGui::Text("Current Section: %s", timelines[timeline_index].sections[current_section].name.c_str());
                        ImGui::PlotLines("Temperature Plate",timelines[timeline_index].logData.temperaturePlate.data(), timelines[timeline_index].logData.time.size(), 0, NULL, 0.0f, 100.0f, ImVec2(0, 80));
                        ImGui::PlotLines("Temperature Sensor", timelines[timeline_index].logData.temperatureSensor.data(), timelines[timeline_index].logData.time.size(), 0, NULL, 0.0f, 100.0f, ImVec2(0, 80));
                        ImGui::PlotLines("Speed", timelines[timeline_index].logData.speed.data(), timelines[timeline_index].logData.time.size(), 0, NULL, 0.0f, 1000.0f, ImVec2(0, 80));
                        ImGui::PlotLines("Viscosity Trend", timelines[timeline_index].logData.viscosity.data(), timelines[timeline_index].logData.time.size(), 0, NULL, 0.0f, 1000.0f, ImVec2(0, 80));
                    }
                    
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();

        ImGui::End();

        // Rendering
        ImGui::Render();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
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