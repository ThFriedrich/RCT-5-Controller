#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <iostream>
#include <string>
#include <vector>

class SerialPort
{
public:
    SerialPort(const std::string &portName, int baudRate);
    ~SerialPort();

    bool open();
    void close();
    bool sendByte(unsigned char byte);
    bool sendCommand(const std::string &command);
    bool readBytes(unsigned char *buffer);
    std::string readString();

    int baudRate;
    std::string portName;
    void checkAvailablePorts();
    std::vector<std::string> availablePorts;

private:
#ifdef _WIN32
    void *handle;
#else
    int fd;
#endif
};
std::vector<std::string> listSerialPorts();
#endif // SERIALPORT_H
