#include <csignal>
#include <iostream>
#include "motor_speed_service.hpp"

MotorSpeedService mss(1000, 100);

void signalHandler(int signum);

int main(int argc, char** argv) {

    signal(SIGINT, signalHandler);

    //TODO process user arguments

    if (mss.init()) {
        mss.start();
        while(mss.is_running());
        return 0;
    }
    else
        return 1;
}

void signalHandler(int signum) {
    std::cout << "Interrupt signal: " << signum << std::endl;

    mss.stop();

    exit(signum);
}
