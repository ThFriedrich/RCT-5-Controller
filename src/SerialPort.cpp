#include "SerialPort.h"
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <string>

SerialPort::SerialPort(const std::string& portName, int baudRate)
    : handle(nullptr), portName(portName), baudRate(baudRate) {}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open() {
    handle = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port" << std::endl;
        return false;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(handle, &dcbSerialParams)) {
        std::cerr << "Error getting serial port state" << std::endl;
        close();
        return false;
    }

    dcbSerialParams.BaudRate = baudRate;
    // dcbSerialParams.ByteSize = 8;
    // dcbSerialParams.StopBits = ONESTOPBIT;
    // dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(handle, &dcbSerialParams)) {
        std::cerr << "Error setting serial port state" << std::endl;
        close();
        return false;
    }

    return true;
}

void SerialPort::close() {
    if (handle != nullptr) {
        CloseHandle(handle);
        handle = nullptr;
    }
}

bool SerialPort::sendCommand(const std::string& command) {
    std::string upperCommand = command;
    std::transform(upperCommand.begin(), upperCommand.end(), upperCommand.begin(), ::toupper);
    upperCommand += " \r\n";
    
    DWORD bytes_written;
    if (!WriteFile(handle, upperCommand.c_str(), upperCommand.size(), &bytes_written, NULL)) {
        std::cerr << "Error writing to serial port" << std::endl;
        return false;
    }
    
    return true;
}

std::string SerialPort::readString() {
    std::string result;
    char buffer;
    DWORD bytes_read;
    
    while (true) {
        if (!ReadFile(handle, &buffer, 1, &bytes_read, NULL)) {
            std::cerr << "Error reading from serial port" << std::endl;
            return result;
        }
        if (bytes_read == 0) {
            break;
        }
        if (buffer == '\n') {
            break;
        }
        result += buffer;
    }
    
    return result;
}

bool SerialPort::readBytes(unsigned char* buffer, size_t size) {
    DWORD bytes_read;
    if (!ReadFile(handle, buffer, size, &bytes_read, NULL)) {
        std::cerr << "Error reading from serial port" << std::endl;
        return false;
    }
    return true;
}

std::vector<std::string> listSerialPorts()
{
    char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
    bool gotPort = false; // in case the port is not found

    std::vector<std::string> ports;

    for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
    {
        std::string str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
        DWORD test = QueryDosDeviceA(str.c_str(), lpTargetPath, 5000);

        // Test the return value and error if any
        if (test  != 0) //QueryDosDevice returns zero if it didn't find an object
        {
            ports.push_back(str);
            //std::cout << str << ": " << lpTargetPath << std::endl;
        }
        if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
        }
    }
    return ports;
}

#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring> 

SerialPort::SerialPort(const std::string& portName, int baudRate)
    : fd(-1), portName(portName), baudRate(baudRate) {}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open() {
    fd = ::open(portName.c_str(), O_RDWR);
    if (fd < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        std::cerr << "Error opening serial port" << std::endl;
        return false;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "Error getting tty attributes" << std::endl;
        printf("Error %i from open: %s\n", errno, strerror(errno));
        close();
        return false;
    }

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag |= CREAD | CLOCAL;

    cfsetispeed(&tty, baudRate);
    cfsetospeed(&tty, baudRate);

    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error setting tty attributes" << std::endl;
        close();
        return false;
    }

    return true;
}

void SerialPort::close() {
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

bool SerialPort::sendByte(unsigned char byte) {
    if (write(fd, &byte, 1) != 1) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool SerialPort::readBytes(unsigned char* buffer, size_t size) {
    int num_bytes = read(fd, buffer, 1);
    if (num_bytes < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool SerialPort::sendCommand(const std::string& command) {
    std::string upperCommand = command;
    std::transform(upperCommand.begin(), upperCommand.end(), upperCommand.begin(), ::toupper);
    upperCommand += " \r\n";
    
    ssize_t bytes_written = write(fd, upperCommand.c_str(), upperCommand.size());
    if (bytes_written < 0) {
        std::cerr << "Error writing to serial port" << std::endl;
        return false;
    }
    
    return true;
}

std::string SerialPort::readString() {
    std::string result;
    char buffer;
    ssize_t bytes_read;
    
    while (true) {
        bytes_read = read(fd, &buffer, 1);
        if (bytes_read < 0) {
            std::cerr << "Error reading from serial port" << std::endl;
            return result;
        }
        if (bytes_read == 0) {
            break;
        }
        if (buffer == '\n') {
            break;
        }
        result += buffer;
    }
    
    return result;
}
#ifndef _WIN32
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

std::vector<std::string> listSerialPorts() {
    std::vector<std::string> ports;
    for (const auto& entry : std::filesystem::directory_iterator("/dev/")) {
        const std::string& path = entry.path().string();
        if (path.find("ttyUSB") != std::string::npos || path.find("ttyS") != std::string::npos || path.find("ttyACM") != std::string::npos) {
            ports.push_back(path);
        }
    }
    return ports;
}
#endif
#endif

