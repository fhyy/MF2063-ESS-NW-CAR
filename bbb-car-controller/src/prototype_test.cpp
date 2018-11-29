#include <iostream>
#include "ess_prototype.cpp"

int main() {

	ESSPrototype proto;

    std::cout << "Attempting to get go status: " << proto.getGoStatus() << std::endl;
    sleep(1);
    std::cout << "Checking motor: " << proto.checkMotor() << std::endl;
    sleep(1);
    std::cout << "Checking speed sensor: " << proto.checkSpeedSensor() << std::endl;
    sleep(1);
    std::cout << "Checking steering: " << proto.checkSteering() << std::endl;
    sleep(1);
    std::cout << "Checking distance sensor: " << proto.checkDistanceSensor() << std::endl;
    sleep(1);
    std::cout << "Checking camera sensor: " << proto.checkCameraSensor() << std::endl;
    sleep(1);
    std::cout << "Attempting to get speed: " << proto.getSpeed() << std::endl;
    sleep(1);
    std::cout << "Attempting to get distance: " << proto.getDistance() << std::endl;
    sleep(1);

    char c = 87;
    std::cout << "Attempting to set min distance: " << (int) c << std::endl;
    proto.setMinDistance(c);

    sleep(1);

    c = 0;
    std::cout << "Attempting to set steering: " << (int) c << std::endl;
    proto.setDirection(c);

    sleep(1);

    c = 100;
    std::cout << "Attempting to set min distance: " << (int) c << std::endl;
    proto.setDirection(c);

    sleep(1);

    c = 200;
    std::cout << "Attempting to set speed: " << (int) c << std::endl;
    proto.setSpeed(c);

    sleep(1);

    c = 0;
    std::cout << "Attempting to set speed: " << (int) c << std::endl;
    proto.setSpeed(c);

    sleep(1);



	return 0;
}
