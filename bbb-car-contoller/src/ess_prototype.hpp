#ifndef ESS_PROTOTYPE_HPP
#define ESS_PROTOTYPE_HPP

#include <vector>
#include "SharedMemory.hpp"
#include "CyclicBuffer.hpp"

class ESSPrototype {
public:
    /**
     * @brief
     * @param ccc
     *
     *
     */
    ESSPrototype();

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
    bool setSpeed(char s, bool prio);

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
    int getSpeed();

    /**
     * @brief
     * @return
     *
     *
     */
    int getDistance();

    /**
     * @brief
     * @return
     *
     *
     */
    bool getGoStatus();

    //TODO Flag getFlag();

    //TODO NetStatus getNetStatus();

private:
    int service_status_;

    CSharedMemory shmMemory_mo;
    Buffer circBuffer_mo;

    CSharedMemory shmMemory_st;
    Buffer circBuffer_st;

    CSharedMemory shmMemory_sp;
    Buffer circBuffer_sp;

    CSharedMemory shmMemory_di;
    Buffer circBuffer_di;

    CSharedMemory shmMemory_go;
    Buffer circBuffer_go;
};

#endif
