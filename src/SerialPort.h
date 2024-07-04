#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <iostream>
#include <string>
#include <vector>

class SerialPort {
public:
    SerialPort(const std::string& portName, int baudRate);
    ~SerialPort();
    
    bool open();
    void close();
    bool sendByte(unsigned char byte);
    bool readBytes(unsigned char* buffer, size_t size);
    

private:
#ifdef _WIN32
    void* handle;
#else
    int fd;
#endif
    std::string portName;
    int baudRate;
};
std::vector<std::string> listSerialPorts(); 
#endif // SERIALPORT_H
