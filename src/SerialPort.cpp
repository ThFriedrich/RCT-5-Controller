#include "SerialPort.h"

#ifdef _WIN32
#include <windows.h>

SerialPort::SerialPort(const std::string& portName, int baudRate)
    : handle(nullptr), portName(portName), baudRate(baudRate) {}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open() {
    handle = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

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

bool SerialPort::sendByte(unsigned char byte) {
    DWORD bytes_written;
    if (!WriteFile(handle, &byte, 1, &bytes_written, NULL)) {
        std::cerr << "Error writing to serial port" << std::endl;
        return false;
    }
    return true;
}

bool SerialPort::readBytes(unsigned char* buffer, size_t size) {
    DWORD bytes_read;
    if (!ReadFile(handle, buffer, size, &bytes_read, NULL)) {
        std::cerr << "Error reading from serial port" << std::endl;
        return false;
    }
    return true;
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
