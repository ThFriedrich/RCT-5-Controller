#ifndef NAMURCOMMANDS_H
#define NAMURCOMMANDS_H

#include <vector>
#include <string>
#include <utility>
#include <map>

class NamurCommands
{
public:
    NamurCommands();

    void printCommands();
    std::string to_string(const std::string &command);
    std::string operator[](int id);
    std::string operator[](size_t id);
    size_t size();

    struct CommandDetails
    {
        std::string description;
        bool requiresValue;
        bool returnsValue;
    };

    const std::vector<std::string> getCommands() const;
    const NamurCommands::CommandDetails &getCommandDetails(const std::string &command) const;

    uint16_t parameter;       // Parameter value e.g. Temperature, Speed, etc.
    std::string responseText; // Response text from the device

private:
    std::map<std::string, CommandDetails> commands;
};
#endif // NAMURCOMMANDS_H
