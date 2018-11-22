#include <csignal>
#include <iostream>
#include "car_ctrl_client.hpp"

CarCTRLClient ccc;

void signalHandler(int signum);

int main(int argc, char** argv) {

    signal(SIGINT, signalHandler);


    if (ccc.init()) {
        ccc.start();
        return 0;
    }
    else
        return 1;
}

void signalHandler(int signum) {
    std::cout << "Interrupt signal: " << signum << std::endl;

    ccc.stop();

    exit(signum);
}
