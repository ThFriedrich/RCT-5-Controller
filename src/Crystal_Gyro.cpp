#include "Crystal_Gyro.h"
#include "SerialPort.h"
#include <vector>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"

static SerialPort *serialPort = nullptr;
static char responseText[256] = "";
std::vector<std::string> availablePorts = listSerialPorts();
static int selectedPortIndex = -1;

void render_window(SDL_Window *window, ImGuiIO &io)
{
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Main loop
    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
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
        
        ImGui::DockSpace(1319);
        static auto first_time = true;
        if (first_time)
        {
            first_time = false;
            ImGui::DockBuilderRemoveNode(1319); // clear any previous layout
            ImGui::DockBuilderAddNode(1319, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderDockWindow("CBED", 1319);
            ImGui::DockBuilderFinish(1319);
        }
        
        // Create ImGui window
        ImGui::Begin("Serial Port Interface");

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
                strcpy(responseText, "Connected to serial port");
            }
            else
            {
                strcpy(responseText, "Failed to connect to serial port");
            }
        }

        // Send signal button
        if (ImGui::Button("Send Signal"))
        {
            if (serialPort)
            {
                unsigned char request = 1;
                if (serialPort->sendByte(request))
                {
                    unsigned char response[6];
                    if (serialPort->readBytes(response, 6))
                    {
                        sprintf(responseText, "Received: %d %d %d %d %d %d", response[0], response[1], response[2], response[3], response[4], response[5]);
                    }
                    else
                    {
                        strcpy(responseText, "Failed to read from serial port");
                    }
                }
                else
                {
                    strcpy(responseText, "Failed to send signal");
                }
            }
            else
            {
                strcpy(responseText, "Serial port not connected");
            }
        }

        // Response text box
        ImGui::InputTextMultiline("Response", responseText, IM_ARRAYSIZE(responseText), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);

        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif
}