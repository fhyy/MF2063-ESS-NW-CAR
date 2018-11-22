#include <csignal>
#include <iostream>
#include "dist_steer_service.hpp"


DistSteerService dss(700);

void signalHandler(int signum);

int main(int argc, char** argv) {

    signal(SIGINT, signalHandler);

    //TODO process user arguments

    if (dss.init()) {
        dss.start();
        while(dss.is_running());
        return 0;
    }
    else
        return 1;
}

void signalHandler(int signum) {
    std::cout << "Interrupt signal: " << signum << std::endl;

    dss.stop();

    exit(signum);
}
