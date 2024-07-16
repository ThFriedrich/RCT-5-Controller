#include "Crystal_Gyro.h"
#include "SerialPort.h"
#include <vector>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "NamurCommands.h"
#include "ImGuiINI.hpp"
#define MINI_CASE_SENSITIVE
#include "ini.h"

static SerialPort *serialPort = nullptr;
std::vector<std::string> availablePorts = listSerialPorts();

static NamurCommands namur;
std::vector<std::string> availableCommands = namur.getCommands();

static int selectedPortIndex = -1;
static int selectedCommandIndex = -1;
static bool use_work_area = true;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable & ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;

void render_window(SDL_Window *window, ImGuiIO &io, SDL_GLContext &gl_context)
{
    // Main loop
    bool done = false;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    
    // INI file settings to restore previous session settings
    mINI::INIFile ini_file("imgui.ini");
    mINI::INIStructure ini_cfg;
    ini_file.read(ini_cfg);

    // Appearance
    static int font_index = 0;
    static int style_index = 0;
    ImGuiINI::check_ini_setting(ini_cfg, "Appearance", "Font", font_index);
    ImGuiINI::set_font(font_index);
    ImGuiINI::check_ini_setting(ini_cfg, "Appearance", "Style", style_index);
    ImGuiINI::set_style(style_index);

    while (!done)

    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
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


        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        // ImGui::DockSpace(1319);
        // static auto first_time = true;
        // if (first_time)
        // {
        //     first_time = false;
        //     ImGui::DockBuilderRemoveNode(1319); // clear any previous layout
        //     ImGui::DockBuilderAddNode(1319, ImGuiDockNodeFlags_DockSpace);
        //     ImGui::DockBuilderDockWindow("CBED", 1319);
        //     ImGui::DockBuilderFinish(1319);
        // }
        // Expose a couple of the available flags. In most cases you may just call BeginTabBar() with no flags (0).

        // Tab Bar

        ImGui::Begin("Serial Port Interface", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNav);
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Appearance"))
            {
                ImGuiINI::ShowFontSelector("Font", font_index, ini_cfg);
                ImGuiINI::ShowStyleSelector("Style", style_index, ini_cfg);
                ImGui::EndMenu();  
            }
        }
        ImGui::EndMainMenuBar();
        // Passing a bool* to BeginTabItem() is similar to passing one to Begin():
        // the underlying bool will be set to false when the tab is closed.
        if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
        {
            ImGui::BeginTabItem("Serial Port Interface", NULL, ImGuiTabItemFlags_None);

            if (ImGui::BeginCombo("Serial Ports", selectedPortIndex >= 0 ? availablePorts[selectedPortIndex].c_str() : "Select a port"))
            {
                for (int i = 0; i < availablePorts.size(); ++i)
                {
                    bool isSelected = (selectedPortIndex == i);
                    if (ImGui::Selectable(availablePorts[i].c_str(), isSelected))
                    {
                        selectedPortIndex = i;
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            // Connect button
            if (ImGui::Button("Connect"))
            {
                if (serialPort)
                {
                    delete serialPort;
                }
                serialPort = new SerialPort(availablePorts[selectedPortIndex].c_str(), 19200); // Change to your serial port

                if (serialPort->open())
                {
                    namur.responseText = "Connected to serial port";
                }
                else
                {
                    namur.responseText = "Failed to connect to serial port";
                }
            }
            if (ImGui::BeginCombo("Commands", selectedCommandIndex >= 0 ? availableCommands[selectedCommandIndex].c_str() : "Select a command"))
            {
                for (int i = 0; i < namur.size(); ++i)
                {
                    bool isSelected = (selectedCommandIndex == i);
                    if (ImGui::Selectable(availableCommands[i].c_str(), isSelected))
                    {
                        selectedCommandIndex = i;
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            if (selectedCommandIndex >= 0)
            {
                auto commandDetails = namur.getCommandDetails(availableCommands[selectedCommandIndex]);
                if (commandDetails.requiresValue)
                {
                    ImGui::SameLine();
                    ImGui::InputScalar("Parameter", ImGuiDataType_U16, &namur.parameter);
                }
                ImGui::Text(commandDetails.description.c_str());

                // Send signal button
                if (ImGui::Button("Send Signal"))
                {
                    if (serialPort)
                    {
                        if (serialPort->sendCommand(namur.to_string(namur[selectedCommandIndex])))
                        {
                            if (commandDetails.returnsValue)
                            {
                                std::string response;
                                namur.responseText = serialPort->readString();
                                if (namur.responseText.size() <= 0)
                                {
                                    namur.responseText = "Failed to read from serial port";
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
            }
            // Response text box
            ImGui::InputTextMultiline("Response", &namur.responseText, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;


        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        // {
        //     SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
        //     SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        // }

        SDL_GL_SwapWindow(window);
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    ini_file.write(ini_cfg);
}