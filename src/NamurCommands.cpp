#include "NamurCommands.h"
#include <iostream>

NamurCommands::NamurCommands() {
    // command, description; bool requiresValue; bool returnsValue;
    commands = {
        {"IN_NAME", {"Read the device name", false, true}},
        {"IN_PV_1", {"Read actual external sensor value", false, true}},
        {"IN_PV_2", {"Read actual hotplate sensor value", false, true}},
        {"IN_PV_4", {"Read stirring speed value", false, true}},
        {"IN_PV_5", {"Read viscosity trend value", false, true}},
        {"IN_SP_1", {"Read rated temperature value", false, true}},
        {"IN_SP_3", {"Read rated set safety temperature value", false, true}},
        {"IN_SP_4", {"Read rated speed value", false, true}},
        {"OUT_SP_1", {"Adjust the set temperature value (x=0...310)", true, false}},
        {"OUT_SP_4", {"Adjust the set speed value (x=0...1500)", true, false}},
        {"START_1", {"Start the heater", false, false}},
        {"STOP_1", {"Stop the heater", false, false}},
        {"START_4", {"Start the motor", false, false}},
        {"STOP_4", {"Stop the motor", false, false}},
        {"RESET", {"Switch to normal operating mode", false, false}},
        {"SET_MODE_A", {"Set operating mode A", false, false}},
        {"SET_MODE_B", {"Set operating mode B", false, false}},
        {"SET_MODE_D", {"Set operating mode D", false, false}},
        {"OUT_SP_12@", {"Setting WD safety limit temperature with set value echo", true, true}},
        {"OUT_SP_42@", {"Setting WD safety limit speed with set value echo", true, true}},
        {"OUT_WD1@", {"Watchdog mode 1: if event WD1 occurs, heating and stirring are off, Er02 is displayed. Set time to m (20 - 1500) seconds.", true, true}},
        {"OUT_WD2@", {"Watchdog mode 2: if event WD2 occurs, speed and temperature limits change. Reset with OUT_WD2@0.", true, true}}
    };
    parameter = 0;
}

size_t NamurCommands::size() {
    return commands.size();
}

std::string NamurCommands::operator[](int id) {
    if (id < 0 || id >= commands.size()) {
        return "Invalid command ID";
    }
    auto it = commands.begin();
    std::advance(it, id);
    return it->first;
}

std::string NamurCommands::operator[](size_t id) {
    if (id < 0 || id >= commands.size()) {
        return "Invalid command ID";
    }
    auto it = commands.begin();
    std::advance(it, id);
    return it->first;
}

const std::vector<std::string> NamurCommands::getCommands() const {

    std::vector<std::string> keys;
    for (const auto& kv : commands) {
        keys.push_back(kv.first);
    }
    return keys;
    }

const NamurCommands::CommandDetails& NamurCommands::getCommandDetails(const std::string &command) const {
    return commands.at(command);
}

std::string NamurCommands::to_string(const std::string &command) {
    std::string result = command;
    NamurCommands::CommandDetails& it = commands.at(command);
    if (it.requiresValue) {
        if (result[result.size() - 1] != '@') {
            result.replace(result.size() - 1, 1, " ");
        }
        result += std::to_string(parameter);
    }
    return result;
}

std::string NamurCommands::get_base_command(const std::string &command) {
    std::string result = command;
    if (result[result.size() - 1] != '@') {
        result.replace(result.size() - 1, 1, " ");
    }
    return result;
}