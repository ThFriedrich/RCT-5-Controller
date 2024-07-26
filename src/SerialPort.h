#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#define PORT_HANDLE void *
#else
#define PORT_HANDLE int
#endif
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

    std::string portName;
    int baudRate;
    void checkAvailablePorts();
    std::vector<std::string> availablePorts;

private:
    PORT_HANDLE handle;
};
std::vector<std::string> listSerialPorts();
#endif // SERIALPORT_H
