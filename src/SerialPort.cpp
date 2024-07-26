#include "SerialPort.h"
#include <algorithm>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>

// Windows implementation
SerialPort::SerialPort(const std::string &portName, int baudRate)
    : handle(nullptr), portName(portName), baudRate(baudRate) {}

SerialPort::~SerialPort()
{
    close();
}

bool SerialPort::open()
{
    DCB dcb;
    COMMTIMEOUTS ct;

    handle = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Error opening serial port" << std::endl;
        return false;
    }

    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(handle, &dcb))
    {
        std::cerr << "Error getting serial port state" << std::endl;
        close();
        return false;
    }

    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;

    if (!SetCommState(handle, &dcb))
    {
        std::cerr << "Error setting serial port state" << std::endl;
        close();
        return false;
    }

    GetCommTimeouts(handle, &ct);
    ct.ReadIntervalTimeout = 100;
    ct.ReadTotalTimeoutMultiplier = 0;
    ct.ReadTotalTimeoutConstant = 100;
    ct.WriteTotalTimeoutMultiplier = 0;
    ct.WriteTotalTimeoutConstant = 100;
    SetCommTimeouts(handle, &ct);

    return true;
}

void SerialPort::close()
{
    if (handle != nullptr)
    {
        CloseHandle(handle);
        handle = nullptr;
    }
}

bool SerialPort::sendCommand(const std::string &command)
{
    std::string upperCommand = command;
    std::transform(upperCommand.begin(), upperCommand.end(), upperCommand.begin(), ::toupper);
    upperCommand += " \r\n";

    DWORD bytes_written;
    if (!WriteFile(handle, upperCommand.c_str(), upperCommand.size(), &bytes_written, NULL))
    {
        std::cerr << "Error writing to serial port" << std::endl;
        return false;
    }

    return true;
}

std::string SerialPort::readString()
{
    std::string result;
    char buffer;
    unsigned long bytes_read;

    while (true)
    {
        if (!ReadFile(handle, &buffer, 1, &bytes_read, NULL))
        {
            std::cerr << "Error reading from serial port" << std::endl;
            return result;
        }
        if (bytes_read == 0)
        {
            break;
        }
        if (buffer == '\n')
        {
            break;
        }
        result += buffer;
    }

    return result;
}

bool SerialPort::readBytes(unsigned char *buffer)
{
    unsigned long bytes_read;
    if (!ReadFile(handle, buffer, 1, &bytes_read, NULL))
    {
        std::cerr << "Error reading from serial port" << std::endl;
        return false;
    }
    return true;
}

std::vector<std::string> listSerialPorts()
{
    char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
    bool gotPort = false;    // in case the port is not found

    std::vector<std::string> ports;

    for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
    {
        std::string str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
        unsigned long test = QueryDosDeviceA(str.c_str(), lpTargetPath, 5000);

        // Test the return value and error if any
        if (test != 0) // QueryDosDevice returns zero if it didn't find an object
        {
            ports.push_back(str);
            // std::cout << str << ": " << lpTargetPath << std::endl;
        }
        if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
        }
    }
    return ports;
}

// Linux implementation
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <filesystem>

SerialPort::SerialPort(const std::string &portName, int baudRate)
    : portName(portName), baudRate(baudRate), handle(-1) {}

SerialPort::~SerialPort()
{
    close();
}

bool SerialPort::open()
{
    int handle = ::open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (handle == -1)
    {
        std::cerr << "Error opening serial port" << std::endl;
        return false;
    }

    struct termios tty;
    if (tcgetattr(handle, &tty) != 0)
    {
        std::cerr << "Error getting terminal attributes" << std::endl;
        ::close(handle);
        return false;
    }

    // Set baud rate
    cfsetispeed(&tty, baudRate);
    cfsetospeed(&tty, baudRate);

    // Set 8N1 (8 data bits, no parity, 1 stop bit)
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // One stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; // 8 data bits

    // Set non-canonical mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;

    // Set timeouts
    tty.c_cc[VMIN] = 0;   // Non-blocking read
    tty.c_cc[VTIME] = 100; // 1 second read timeout

    // Apply the settings
    if (tcsetattr(handle, TCSANOW, &tty) != 0)
    {
        std::cerr << "Error setting terminal attributes" << std::endl;
        ::close(handle);
        return false;
    }

    // Flush the input and output buffers
    // if (tcflush(handle, TCIOFLUSH) != 0) {
    //     std::cerr << "Error flushing buffers: " << strerror(errno) << std::endl;
    //     ::close(handle);
    //     return false;
    // }

    // Store the handle for later use
    this->handle = handle;
    return true;
}

void SerialPort::close()
{
    if (handle >= 0)
    {
        ::close(handle);
        handle = -1;
    }
}

bool SerialPort::sendByte(unsigned char byte)
{
    if (write(handle, &byte, 1) != 1)
    {
        printf("Error %i from sendByte: %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool SerialPort::readBytes(unsigned char *buffer)
{
    int num_bytes = read(handle, buffer, 1);
    if (num_bytes < 0)
    {
        printf("Error %i from readBytes: %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool SerialPort::sendCommand(const std::string &command)
{
    std::string upperCommand = command;
    std::transform(upperCommand.begin(), upperCommand.end(), upperCommand.begin(), ::toupper);
    upperCommand += " \r\n";

    ssize_t bytes_written = write(handle, upperCommand.c_str(), upperCommand.size());
    if (bytes_written < 0)
    {
        printf("Error %i from sendCommand: %s\n", errno, strerror(errno));
        return false;
    }

    return true;
}

std::string SerialPort::readString() {
    if (handle == -1) {
        std::cerr << "Serial port not open" << std::endl;
        return "";
    }

    char buffer[256];
    std::string result;
    int bytesRead;

    while (true) {
        bytesRead = ::read(handle, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result += buffer;
            if (result.find('\n') != std::string::npos) {
                break;
            }
        } else if (bytesRead == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available, continue reading
                usleep(1000); // Sleep for 1ms
                continue;
            } else {
                std::cerr << "Error reading from serial port: " << strerror(errno) << std::endl;
                break;
            }
        } else {
            // No more data
            break;
        }
    }

    return result;
}

std::vector<std::string> listSerialPorts()
{
    std::vector<std::string> ports;
    for (const auto &entry : std::filesystem::directory_iterator("/dev/"))
    {
        const std::string &path = entry.path().string();
        if (path.find("ttyUSB") != std::string::npos || path.find("ttyS") != std::string::npos || path.find("ttyACM") != std::string::npos)
        {
            ports.push_back(path);
        }
    }
    return ports;
}

#endif
