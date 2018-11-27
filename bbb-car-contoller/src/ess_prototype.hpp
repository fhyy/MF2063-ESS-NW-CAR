#ifndef ESS_PROTOTYPE_HPP
#define ESS_PROTOTYPE_HPP

#include "car_ctrl_client.hpp"

class ESSPrototype {
public:
    /**
     * @brief
     * @param ccc
     *
     *
     */
    ESSPrototype(CarCTRLClient& ccc);

    //TODO bool boot();

    //TODO bool shutdown();

    /**
     * @brief
     * @return
     *
     *
     */
    bool checkMotor();

    /**
     * @brief
     * @return
     *
     *
     */
    bool checkSpeedSensor();

    /**
     * @brief
     * @return
     *
     *
     */
    bool checkSteering();

    /**
     * @brief
     * @return
     *
     *
     */
    bool checkDistanceSensor();

    /**
     * @brief
     * @return
     *
     *
     */
    bool setMinDistance(char d, bool prio);

    /**
     * @brief
     * @param s
     * @param prio
     * @return
     *
     *
     */
    bool setSpeed(char d, bool prio);

    /**
     * @brief
     * @param s
     * @param a
     * @param prio
     * @return
     *
     *
     */
    bool setSpeed(char s, char a, bool prio);

    /**
     * @brief
     * @param d
     * @param prio
     * @return
     *
     *
     */
    bool setDirection(char d, bool prio);

    /**
     * @brief
     * @return
     *
     *
     */
    char getSpeed();

    /**
     * @brief
     * @return
     *
     *
     */
    //TODO char* getDistance();

    //TODO Flag getFlag();

    //TODO NetStatus getNetStatus();

private:
    CarCTRLClient& carCTRL_;
};

#endif
