#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string>
#include <fcntl.h>

#include <iostream>
using namespace std;

class PortChat
{
private:
    int _serialPort;

public:
    PortChat(string portName) {
        _serialPort = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

        if (_serialPort == -1) {
            std::cout << "Failed to open port " << portName << endl;
            exit(-1);
        }

        struct termios port_settings;

        cfsetispeed(&port_settings, B19200);
        cfsetospeed(&port_settings, B19200);

        tcsetattr(_serialPort, TCSANOW, &port_settings);
    }

    ~PortChat() {
        close(_serialPort);
    }

    void Write(string cmd) {
        write(_serialPort, cmd.c_str(), cmd.length());
    }

    string Read(bool *hasRead, int length) {
        char cmdIn[length];
        int ret = read(_serialPort, cmdIn, length);

        if (ret == -1) {
            std::cout << "Error in reading, please re-start\n";
            exit(-1);
        }

        if (ret > 0) {
            *hasRead = true;

            cmdIn[ret - 1] = 0;
            string cmdRet = string(cmdIn);

            return cmdRet;
        } else {
            *hasRead = false;
            return "";
        }
    }
};
