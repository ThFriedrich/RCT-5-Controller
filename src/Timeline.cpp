#include "TimeLine.h"
#include "beeper.h"

void LogData::addData(float t, float tp, float ts, float s, float v)
{
    time.push_back(t);
    temperaturePlate.push_back(tp);
    temperatureSensor.push_back(ts);
    speed.push_back(s);
    viscosity.push_back(v);
}

TimeLine::~TimeLine()
{
    // Ensure the thread is joined on destruction
    if (communication_thread != nullptr && communication_thread->joinable())
    {
        communication_thread->join();
    }
}

void TimeLine::addSection(const Section &section)
{
    sections.push_back(section);
}

void Section::sound_beep()
{
    std::thread t1 = std::thread(Beeper::super_mario_level_finshed, 0.75);
    t1.detach();
}

void TimeLine::execute()
{
    communication_thread = new std::thread([this]
                                           { execute_thread(); });
    running = true;
}

void TimeLine::execute_thread()
{
    for (Section &section : sections)
    {
        section.execute_section();
        current_section++;
    }
}

void TimeLine::stop()
{
    b_stop = true;
    if (communication_thread != nullptr && communication_thread->joinable())
    {
        communication_thread->join();
    }
}
void Section::compile_section()
{
    // determine if ramping is needed
    bool b_ramp = temperature[0] != temperature[1] || speed[0] != speed[1];
    // Value set interval in milliseconds
    interval = b_ramp ? 100 : duration * 1000;
    // Compute amount of steps
    size_t steps = b_ramp ? duration * 10 : 1;
    // Resize vectors for temperatures and speeds
    temperatures.resize(steps);
    speeds.resize(steps);
    // Compute step size for temperature and speed
    float temperature_step = static_cast<float>(temperature[1] - temperature[0]) / static_cast<float>(steps);
    float speed_step = static_cast<float>(speed[1] - speed[0]) / static_cast<float>(steps);
    // Fill vectors with values
    for (size_t i = 0; i < steps; i++)
    {
        temperatures[i] = static_cast<float>(temperature[0]) + i * temperature_step;
        speeds[i] = static_cast<float>(speed[0]) + i * speed_step;
    }
}

void Section::execute_section()
{
    compile_section();
    bool b_log = (timeline->logTemperaturePlate || timeline->logSpeed || timeline->logViscosity || timeline->logTemperatureSensor) && timeline->logFilePath != "";
    std::ofstream logFile;
    if (b_log)
    {
        logFile.open(timeline->logFilePath, std::ios::app);
        logFile << "Section: " << name << std::endl;
        logFile << "Duration: " << duration << " s" << std::endl;
        logFile << "Temperature: " << temperature[0] << " -> " << temperature[1] << " °C" << std::endl;
        logFile << "Speed: " << speed[0] << " -> " << speed[1] << " RPM" << std::endl;
        logFile << std::endl;
    }

    if (preSectionCommands.size() > 0)
    {
        if (b_log)
        {
            logFile << "Pre-section commands:" << std::endl;
        }
        for (const std::string &command : preSectionCommands)
        {
            timeline->rct->send_signal(command);
            if (b_log)
            {
                logFile << command << "\t" << timeline->rct->get_response() << std::endl;
            }
        }
    }

    // Write header for log file numeric data
    if (b_log)
    {
        logFile << std::endl
                << "LOGDATA" << std::endl;
        logFile << "Time\t\t";
        if (timeline->logSpeed)
        {
            logFile << "Speed\t\t";
        }
        if (timeline->logTemperaturePlate)
        {
            logFile << "Temperature Plate\t\t";
        }
        if (timeline->logTemperatureSensor)
        {
            logFile << "Temperature Sensor\t\t";
        }
        if (timeline->logViscosity)
        {
            logFile << "Viscosity\t\t";
        }
        logFile << std::endl;
        logFile.close();
    }

    std::chrono::time_point<std::chrono::system_clock> t_start = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> t_now;
    std::chrono::time_point<std::chrono::system_clock> t_last_interval;
    std::chrono::time_point<std::chrono::system_clock> t_last_log;
    size_t ms_duration = duration * 1000;
    size_t logInterval_ms = timeline->logInterval * 1000;
    size_t ms_passed = 0;
    size_t ms_passed_interval = 0;
    size_t ms_passed_log = 0;
    size_t step = 0;

    while (ms_passed < ms_duration)
    {
        if (timeline->b_stop)
        {
            break;
        }
        t_now = std::chrono::system_clock::now();
        ms_passed = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_start).count();
        ms_passed_interval = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_last_interval).count();

        if (step == 0 || ms_passed_interval >= interval)
        {
            timeline->rct->send_signal("OUT_SP_1 " + std::to_string(std::round(temperatures[step])));
            timeline->rct->send_signal("OUT_SP_4 " + std::to_string(std::round(speeds[step])));
            step++;
            t_last_interval = std::chrono::system_clock::now();
        }

        ms_passed_log = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_last_log).count();
        if (b_log && ms_passed_log >= logInterval_ms)
        {
            logFile.open(timeline->logFilePath, std::ios::app);
            logFile << ftos(static_cast<float>(ms_passed) / 1000, 2) << "\t\t";
            timeline->logData.time.push_back(static_cast<float>(ms_passed) / 1000);
            if (timeline->logSpeed)
            {
                timeline->rct->send_signal("IN_PV_4");
                std::string response = timeline->rct->get_response();
                logFile <<response << "\t\t";
                timeline->logData.speed.push_back(std::stof(response));
            }
            if (timeline->logTemperaturePlate)
            {
                timeline->rct->send_signal("IN_PV_1");
                std::string response = timeline->rct->get_response();
                logFile << response << "\t\t";
                timeline->logData.temperaturePlate.push_back(std::stof(response));
            }
            if (timeline->logTemperatureSensor)
            {
                timeline->rct->send_signal("IN_PV_2");
                std::string response = timeline->rct->get_response();
                logFile << response << "\t\t";
                timeline->logData.temperatureSensor.push_back(std::stof(response));
            }
            if (timeline->logViscosity)
            {
                timeline->rct->send_signal("IN_PV_5");
                std::string response = timeline->rct->get_response();
                logFile << response << "\t\t";
                timeline->logData.viscosity.push_back(std::stof(response));
            }
            logFile << std::endl;
            logFile.close();
            t_last_log = std::chrono::system_clock::now();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!timeline->b_stop)
    {

        if (postSectionCommands.size() > 0)
        {
            if (b_log)
            {
                logFile.open(timeline->logFilePath, std::ios::app);
                logFile << "Post-section commands:" << std::endl;
            }
            for (const std::string &command : postSectionCommands)
            {
                timeline->rct->send_signal(command);
                if (b_log)
                {
                    logFile << command << "\t" << timeline->rct->get_response() << std::endl;
                }
            }
        }
        if (b_beep)
        {
            sound_beep();
        }
        if (wait)
        {
            timeline->waiting = true;
            while (timeline->waiting)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    if (logFile.is_open())
    {
        logFile << std::endl
                << std::endl;
        logFile.close();
    }
}