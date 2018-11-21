#include "ess_prototype.hpp"

ESSPrototype::ESSPrototype(CarCTRLClient& ccc) :
    carCTRL_(ccc)
    {}

//TODO bool EssPrototype::boot() {}

//TODO bool EssPrototype::shutdown() {}

bool ESSPrototype::checkMotor() {
    return carCTRL_.check_availability(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);
}

bool ESSPrototype::checkSpeedSensor() {
    return carCTRL_.check_availability(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);
}

bool ESSPrototype::checkSteering() {
    return carCTRL_.check_availability(STEER_SERVICE_ID, STEER_INSTANCE_ID);
}

bool ESSPrototype::checkDistanceSensor() {
    return carCTRL_.check_availability(DIST_SERVICE_ID, DIST_INSTANCE_ID);
}

bool ESSPrototype::setSpeed(char s, bool prio) {
    if (!checkMotor()) {
        return false;
    }
    else {
        carCTRL_.send_motor_req(s, prio);
    }
}

bool ESSPrototype::setSpeed(char s, char a, bool prio) {
    if (!checkMotor()) {
        return false;
    }
    else {
        carCTRL_.send_motor_req(s, a, prio);
    }
}

bool ESSPrototype::setDirection(char d, bool prio) {
    if (!checkSteering()) {
        return false;
    }
    else {
        carCTRL_.send_steer_req(d, prio);
    }
}

char ESSPrototype::getSpeed() {
    if (!checkSpeedSensor()) {
        return 0;
    } else {
        return carCTRL_.pop_speed();
    }
}


/* TODO char ESSPrototype::getDistance() {
    if (!checkDistanceSensor()) {
        return {0};
    } else {
        return carCTRL_.pop_distance();
    }
}*/
